//
// Created by Dmitrii Borisov on 15.10.2022.
//

#include "YandexSttSession.h"
#include "ytg_bindings.h"

#include <grpc++/create_channel.h>
#include <memory>
#include <google/protobuf/util/json_util.h>

#ifdef __cplusplus
extern "C" {
#endif

void *create_yandex_stt_session(ysg_config_t *config, unsigned sample_rate, void *pToken) {
	if (!std::thread::hardware_concurrency())
		return nullptr;
	auto token = reinterpret_cast<cYandexGrpcIamToken *>(pToken);
	std::string host{config->sttReceiverHostName}, t_ = std::move(token->GetToken());
	try {
		auto session = new cYandexSttSession(host, t_, sample_rate);
		return session;
	} catch (std::exception &e) {
		return nullptr;
	}
}

switch_status_t check_yandex_stt_results(void *session) {
	auto s_ = reinterpret_cast<cYandexSttSession *>(session);
	return s_->HasNewResults() ? SWITCH_STATUS_SUCCESS : SWITCH_STATUS_FALSE;
}

switch_status_t get_yandex_stt_results(void *session, char **results, switch_memory_pool_t *pool) {
	auto s_ = reinterpret_cast<cYandexSttSession *>(session);
	if (!s_)
		return SWITCH_STATUS_GENERR;
	auto alts = s_->GetAlternatives();
	if (alts == nullptr)
		return SWITCH_STATUS_FALSE;
	string res;
	for (const auto &alt: *alts)
		res += alt + "\n";
	if (!res.empty())
		*results = switch_safe_strdup(res.c_str());
	return s_->FinalResultsReceived() ? SWITCH_STATUS_SUCCESS : SWITCH_STATUS_MORE_DATA;
} ;

switch_status_t destroy_yandex_stt_session(void **session) {
	auto s_ = reinterpret_cast<cYandexSttSession *>(*session);
	s_->Complete();
	if (s_->HasNewResults()) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "ASR session has results, but going to be "
		                                                        "destroyed:\n");
		auto alts = s_->GetAlternatives();
		if (alts) {
			string res;
			for (const auto &alt: *alts)
				res += alt + "\n";
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "%s\n", res.c_str());
		}
	}
	delete s_;
	*session = nullptr;
	return SWITCH_STATUS_SUCCESS;
}

switch_status_t feed_to_yandex_stt_session(void *session, void *data, unsigned int len) {
	auto s_ = reinterpret_cast<cYandexSttSession *>(session);
	cYandexSttSession::Sample smpl{len, static_cast<char *>(data)};
	try {
		(*s_) << smpl;
		return SWITCH_STATUS_SUCCESS;
	} catch (exception &) {
		return SWITCH_STATUS_BREAK;
	}
}

#ifdef __cplusplus
}
#endif

using namespace google::protobuf::util;

cYandexSttSession::cYandexSttSession(const std::string &HostName, std::string &Bearer, unsigned SampleRate)
	: sampleRate_(SampleRate), buf_(), maxBufLen_(SampleRate * L16PCM_SAMPLE_SIZE) {
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
	req.mutable_session_options()->mutable_eou_classifier()->mutable_default_classifier()
		->set_type(DefaultEouClassifier_EouSensitivity_EouSensitivity_MAX);

	if (!rw_->Write(req))
		throw YandexSessionException("Error while configuring Yandex.Cloud.Speechki.STT connection");

	reader = new std::thread(cYandexSttSessionReader_(rw_, this));
}

cYandexSttSession::~cYandexSttSession() {
	Complete();
}

std::string::size_type cYandexSttSession::write_() {
	StreamingRequest req;
	auto len_ = buf_.length();
	req.mutable_chunk()->set_data(buf_);
	if (!rw_->Write(req))
		throw YandexSessionException("Error while trying to post sound chunk to Yandex.Cloud.Speechki.STT");
	buf_.clear();
	return len_;
}

cYandexSttSession &cYandexSttSession::operator<<(cYandexSttSession::Sample sample) {
	if (buf_.size() >= maxBufLen_)
		write_();
	buf_ += string(sample.data, sample.len);
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

std::unique_ptr<std::vector<std::string>> cYandexSttSession::GetAlternatives() {
	if (currentAlternativesRead_)
		return nullptr;
	std::unique_lock<std::mutex> l(respLock_);
	currentAlternativesRead_ = true;
	return std::make_unique<std::vector<std::string>>(alternatives_);
}

void cYandexSttSession::cYandexSttSessionReader_::operator()() {
	StreamingResponse response;
	while (rw_->Read(&response)) {
		string json_;
		JsonPrintOptions opts;
		opts.add_whitespace = true;
		switch (response.Event_case()) {
			case StreamingResponse::kPartial: {
				if (self->final_)
					continue;
				std::unique_lock<std::mutex> l(self->respLock_);
				self->alternatives_.clear();
				for (const auto &alt: response.partial().alternatives()) {
					if (MessageToJsonString(alt, &json_, opts) == status_internal::OkStatus())
						self->alternatives_.push_back(json_);
				}
				self->currentAlternativesRead_ = false;
			}
				break;
			case StreamingResponse::kFinal: {
				if (self->final_)
					continue;
				std::unique_lock<std::mutex> l(self->respLock_);
				self->alternatives_.clear();
				for (const auto &alt: response.final().alternatives()) {
					if (MessageToJsonString(alt, &json_, opts) == status_internal::OkStatus())
						self->alternatives_.push_back(json_);
				}
				self->final_ = true;
				self->currentAlternativesRead_ = false;
			}
				break;
			case StreamingResponse::kFinalRefinement: {
				if (self->finalRefined_ || !self->normalization_)
					continue;
				std::unique_lock<std::mutex> l(self->respLock_);
				self->alternatives_.clear();
				for (const auto &alt: response.final_refinement().normalized_text().alternatives()) {
					if (MessageToJsonString(alt, &json_, opts) == status_internal::OkStatus())
						self->alternatives_.push_back(json_);
				}
				self->finalRefined_ = true;
				self->currentAlternativesRead_ = false;
			}
				break;
			case StreamingResponse::kEouUpdate:
			case StreamingResponse::kStatusCode:
			case StreamingResponse::EVENT_NOT_SET:
				break;
		}
	}
	rw_->Finish();
}
