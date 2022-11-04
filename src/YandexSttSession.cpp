//
// Created by dbori on 15.10.2022.
//

#include "YandexSttSession.h"

#include <grpc++/create_channel.h>
#include <utility>

YandexSttSession::YandexSttSession(const std::string &HostName, std::string &Bearer, std::string LegUuid,
                                   unsigned SampleRate, unsigned int bufferLen)
	: sampleRate_(SampleRate), legUuid(std::move(LegUuid)),
	  enc_(speechkit::stt::v3::RawAudio_AudioEncoding_LINEAR16_PCM)
{
	auto chan_ = CreateChannel(HostName, grpc::SslCredentials(grpc::SslCredentialsOptions()));
	stub_ = Recognizer::NewStub(chan_);

	StreamingRequest req;
	req.mutable_session_options()->mutable_recognition_model()->mutable_audio_format()->mutable_raw_audio()
		->set_sample_rate_hertz(SampleRate);
	req.mutable_session_options()->mutable_recognition_model()->mutable_audio_format()->mutable_raw_audio()
		->set_audio_channel_count(1);
	req.mutable_session_options()->mutable_recognition_model()->mutable_audio_format()->mutable_raw_audio()
		->set_audio_encoding(enc_);
	req.mutable_session_options()->mutable_recognition_model()->mutable_language_restriction()
		->set_language_code(LanguageRestrictionOptions_LanguageRestrictionType_WHITELIST, "ru-RU");
	req.mutable_session_options()->mutable_recognition_model()
		->set_audio_processing_type(speechkit::stt::v3::RecognitionModelOptions_AudioProcessingType_REAL_TIME);

	context_.AddMetadata("Authorization", "Bearer " + Bearer);

	rw_ = std::shared_ptr<grpc::ClientReaderWriter<StreamingRequest, StreamingResponse>>(
		stub_->RecognizeStreaming(&context_));

	if (!rw_->Write(req))
		throw YandexSessionException("Error while configuring Yandex.Cloud.Speechki.STT connection");
}