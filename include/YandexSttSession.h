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
#include <thread>

#include "YandexIamToken.h"

#define L16PCM_SAMPLE_SIZE 2

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using namespace speechkit::stt::v3;

class YandexSessionException : public std::exception {
	const std::string reason_;
public:
	explicit YandexSessionException(std::string reason) : reason_(std::move(reason)) {}

	const char *what() { return reason_.data(); }
};

class cYandexSttSession : public Recognizer::AsyncService {

	class cYandexSttSessionReader_ {
		std::shared_ptr<grpc::ClientReaderWriter<StreamingRequest, StreamingResponse>> rw_;
		cYandexSttSession *self;
	public:
		cYandexSttSessionReader_(std::shared_ptr<grpc::ClientReaderWriter<StreamingRequest, StreamingResponse>> &rw_,
		                         cYandexSttSession *p)
			: rw_(rw_), self(p) {};

		void operator()();
	};

	std::unique_ptr<Recognizer::Stub> stub_;
	ClientContext context_;
	std::shared_ptr<grpc::ClientReaderWriter<StreamingRequest, StreamingResponse>> rw_;
//	std::ostringstream buf_;
	std::string buf_;
	std::thread *reader;
	unsigned sampleRate_, maxBufLen_;
	std::mutex respLock_;
	bool running_{true};
	std::vector<std::string> alternatives_{};
	bool final_{false};
	bool finalRefined_{false};
	bool normalization_{true};
	bool currentAlternativesRead_{true};

	std::string::size_type write_();

public:
	typedef struct {
		long len;
		char *data;
	} Sample;

	cYandexSttSession() = delete;

	cYandexSttSession(cYandexSttSession &) = delete;

	/**
	 * Constructor for L16 streaming
	 * @param HostName - host where to connect with gRPC and stream sound
	 * @param Bearer - Yandex Iam token
	 * @param SampleRate - counts of samples per second
	 *
	 * todo add normalization to module and constructor parameters
	 */
	cYandexSttSession(const std::string &HostName, std::string &Bearer, unsigned SampleRate);

	~cYandexSttSession() override;

	cYandexSttSession &operator<<(Sample sample);

	std::unique_ptr<std::vector<std::string>> GetAlternatives();

	void Complete();

	bool HasNewResults() {
		std::unique_lock<std::mutex> l_(respLock_);
		return !currentAlternativesRead_;
	}

	bool FinalResultsReceived() {
		std::unique_lock<std::mutex> l_(respLock_);
		return final_ && ((normalization_ && finalRefined_) || !normalization_);
	}
};

#endif//MOD_YANDEX_TRANSCRIBE_YANDEXSTTSESSION_H
