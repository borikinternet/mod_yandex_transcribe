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

	class cYandexAudioStreamException_ : public std::exception {
	};

	class cYandexAudioStream_ : public std::ostringstream {
		class sentry : public std::ostringstream::sentry {
		public:
			explicit sentry(std::ostringstream &s)
				: std::ostringstream::sentry(s) {
				if (s.str().size() == s.str().capacity())
					throw cYandexAudioStreamException_();
			};
		};

	public:
		cYandexAudioStream_(const std::string &s_, openmode m_) : std::ostringstream(s_, m_) {}
	};

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
	std::string legUuid;
	ClientContext context_;
	std::shared_ptr<grpc::ClientReaderWriter<StreamingRequest, StreamingResponse>> rw_;
	cYandexAudioStream_ buf_;
	std::thread *reader;
	unsigned sampleRate_;
	std::mutex respLock_;
	StreamingResponse response_;
	bool running_ {true};
	std::vector<std::string> alternatives_{};
	bool final_ {false};

	std::string::size_type write_();

	void setResponse(StreamingResponse resp) {
		std::unique_lock<std::mutex> l(respLock_);
		response_ = std::move(resp);
	};

public:
	typedef struct {
		long len;
		char *data;
	} Sample;

	/**
	 * Constructor for L16 streaming
	 * @param HostName - host where to connect with gRPC and stream sound
	 * @param Bearer - Yandex Iam token
	 * @param LegUuid - freeswitch leg unique id
	 * @param SampleRate - counts of samples per second
	 */
	cYandexSttSession(const std::string &HostName, std::string &Bearer, std::string LegUuid,
	                  unsigned SampleRate);

	~cYandexSttSession() override;

	cYandexSttSession &operator<<(Sample sample);

	// todo refactor to parse results into event view
	StreamingResponse GetLastResponse() {
		std::unique_lock<std::mutex> l(respLock_);
		return response_;
	}

	const std::vector<std::string> &GetAlternatives() {
		std::unique_lock<std::mutex> l(respLock_);
		return alternatives_;
	}

	void Complete();
};

#endif//MOD_YANDEX_TRANSCRIBE_YANDEXSTTSESSION_H
