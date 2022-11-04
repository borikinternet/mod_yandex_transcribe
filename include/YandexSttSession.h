//
// Created by dbori on 15.10.2022.
//

#ifndef MOD_YANDEX_TRANSCRIBE_YANDEXSTTSESSION_H
#define MOD_YANDEX_TRANSCRIBE_YANDEXSTTSESSION_H

#include <grpcpp/security/credentials.h>
#include <yandex/cloud/ai/stt/v3/stt_service.grpc.pb.h>
#include <yandex/cloud/ai/stt/v3/stt_service.pb.h>

#include <utility>
#include <sstream>

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using namespace speechkit::stt::v3;

class YandexSessionException : public std::exception {
	const std::string _r;
public:
	explicit YandexSessionException(std::string reason) : _r(std::move(reason)) {}

	const char *what() { return _r.data(); }
};


class YandexSttSession : public Recognizer::AsyncService {
	std::unique_ptr<Recognizer::Stub> stub_;
	std::string legUuid;
	unsigned sampleRate_;
	ClientContext context_;
	std::shared_ptr<grpc::ClientReaderWriter<StreamingRequest, StreamingResponse>> rw_;
	RawAudio_AudioEncoding enc_;
	char *buf;
public:
	typedef struct {
		long len_;
		void *data;
	} Sample;

	/**
	 * Constructor for L16 streaming
	 * @param HostName - host where to connect with gRPC and stream sound
	 * @param Bearer - Yandex Iam token
	 * @param LegUuid - freeswitch leg unique id
	 * @param SampleRate - counts of samples per second
	 * @param bufferLen - buffer size, in seconds
	 */
	YandexSttSession(const std::string &HostName, std::string &Bearer, std::string LegUuid,
	                 unsigned SampleRate, unsigned int bufferLen);

	YandexSttSession &operator<<(Sample sample);
};

#endif//MOD_YANDEX_TRANSCRIBE_YANDEXSTTSESSION_H
