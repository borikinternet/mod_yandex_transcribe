//
// Created by Dmitrii Borisov on 15.10.2022.
//

#include "YandexSttSession.h"

#include <grpc++/create_channel.h>
#include <utility>

cYandexSttSession::cYandexSttSession(const std::string &HostName, std::string &Bearer, std::string LegUuid,
                                     unsigned SampleRate)
	: legUuid(std::move(LegUuid)), sampleRate_(SampleRate),
	  buf_(std::string(SampleRate * L16PCM_SAMPLE_SIZE, '\0'), std::ios_base::app | std::ios_base::binary) {
	auto chan_ = CreateChannel(HostName, grpc::SslCredentials(grpc::SslCredentialsOptions()));
	stub_ = Recognizer::NewStub(chan_);

	context_.AddMetadata({"authorization"}, {"Bearer " + Bearer});
	rw_ = std::shared_ptr<grpc::ClientReaderWriter<StreamingRequest, StreamingResponse>>(
		stub_->RecognizeStreaming(&context_));

	StreamingRequest req;
	req.mutable_session_options()->mutable_recognition_model()->mutable_audio_format()->mutable_raw_audio()
		->set_sample_rate_hertz(SampleRate);
	req.mutable_session_options()->mutable_recognition_model()->mutable_audio_format()->mutable_raw_audio()
		->set_audio_channel_count(1);
	req.mutable_session_options()->mutable_recognition_model()->mutable_audio_format()->mutable_raw_audio()
		->set_audio_encoding(RawAudio_AudioEncoding_LINEAR16_PCM);
	req.mutable_session_options()->mutable_recognition_model()->mutable_text_normalization()
		->set_text_normalization(TextNormalizationOptions_TextNormalization_TEXT_NORMALIZATION_ENABLED);
	req.mutable_session_options()->mutable_recognition_model()->mutable_language_restriction()
		->mutable_language_code()->Add("ru-RU");
	req.mutable_session_options()->mutable_recognition_model()->mutable_language_restriction()
		->set_restriction_type(speechkit::stt::v3::LanguageRestrictionOptions_LanguageRestrictionType_WHITELIST);
	req.mutable_session_options()->mutable_recognition_model()
		->set_audio_processing_type(speechkit::stt::v3::RecognitionModelOptions_AudioProcessingType_REAL_TIME);

	if (!rw_->Write(req))
		throw YandexSessionException("Error while configuring Yandex.Cloud.Speechki.STT connection");

	reader = new std::thread(cYandexSttSessionReader_(rw_, this));
}

cYandexSttSession::~cYandexSttSession() {
	Complete();
}

std::string::size_type cYandexSttSession::write_() {
	StreamingRequest req;
	auto len_ = buf_.str().length();
	req.mutable_chunk()->set_data(buf_.str());
	if (!rw_->Write(req))
		throw YandexSessionException("Error while trying to post sound chunk to Yandex.Cloud.Speechki.STT");
	buf_.str().clear();
	buf_.str().resize(sampleRate_ * L16PCM_SAMPLE_SIZE);
	return len_;
}

cYandexSttSession &cYandexSttSession::operator<<(cYandexSttSession::Sample sample) {
	try {
		buf_.write(sample.data, sample.len);
	} catch (cYandexAudioStreamException_ &) {
		write_();
		buf_.write(sample.data, sample.len);
	}
	return *this;
}

void cYandexSttSession::Complete() {
	if (!running_)
		return;
	write_();
	rw_->WritesDone();
	reader->join();
	running_ = false;
}

void cYandexSttSession::cYandexSttSessionReader_::operator()() {
	StreamingResponse response;
	while (rw_->Read(&response)) {
		switch (response.Event_case()) {
			case StreamingResponse::kPartial: {
				if (self->final_)
					continue;
				std::unique_lock<std::mutex> l(self->respLock_);
				self->alternatives_.clear();
				for (const auto &alt: response.partial().alternatives())
					self->alternatives_.push_back(alt.text());
				// todo fire an event to publish partial recognition results
			}
				break;
			case StreamingResponse::kFinal: {
				std::unique_lock<std::mutex> l(self->respLock_);
				self->alternatives_.clear();
				for (const auto &alt: response.final().alternatives())
					self->alternatives_.push_back(alt.text());
				if (!self->final_)
					self->final_ = true;
				// todo fire an event to stop/restart recognition
			}
				break;
			case StreamingResponse::kFinalRefinement: {
				std::unique_lock<std::mutex> l(self->respLock_);
				self->alternatives_.clear();
				for (const auto &alt: response.final_refinement().normalized_text().alternatives())
					self->alternatives_.push_back(alt.text());
				// todo fire an event to stop/restart recognition
			}
				break;
			case StreamingResponse::kEouUpdate:
			case StreamingResponse::kStatusCode:
			case StreamingResponse::EVENT_NOT_SET:
				break;
		}
		self->setResponse(response);
	}
	rw_->Finish();
}
