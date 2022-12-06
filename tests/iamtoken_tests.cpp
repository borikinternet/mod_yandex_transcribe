//
// Created by dbori on 08.10.2022.
//
#define BOOST_TEST_MODULE iamtoken_tests

#define TEST 1

#include "tests/config.h"
#include <boost/test/unit_test.hpp>
#include <fstream>

#include "YandexIamToken.h"
#include "YandexSttSession.h"
#include <date/date.h>

struct fixt {
	ysg_config_t config{"aje95nign4v8tj6cl49f", "ajenj092br8692njemn0",
	                    PROJECT_SOURCE_DIR "/conf/speechkit_id.pub",
	                    PROJECT_SOURCE_DIR "/conf/speechkit_id",
	                    "https://iam.api.cloud.yandex.net/iam/v1/tokens",
	                    "stt.api.cloud.yandex.net:443"};
};

BOOST_AUTO_TEST_CASE(date_tests) {
	stringstream expiresAtStr;
	expiresAtStr << "1970-01-01T01:00:00.000000000Z";
	date::sys_seconds tps;
	expiresAtStr >> date::parse("%4FT%H:%M:%12S%Z", tps);
	BOOST_TEST(tps.time_since_epoch().count() == 3600);
}

BOOST_FIXTURE_TEST_CASE(connection_test, fixt)
{
	BOOST_TEST_MESSAGE(config.privKeyFile);
	BOOST_TEST_MESSAGE(config.pubKeyFile);
	auto token_ = new cYandexGrpcIamToken(config);
	BOOST_TEST(token_);
	BOOST_TEST(token_->Renew());
	BOOST_TEST(token_->GetToken().length());
	BOOST_TEST(token_->Renew());
	BOOST_TEST(token_->GetToken().length());
	delete token_;
	const ysg_config_t incorrect_config({config.accId, "zzz", config.pubKeyFile,
	                                     config.privKeyFile, config.getTokenUrl});
	auto bad_token = new cYandexGrpcIamToken(incorrect_config);
	BOOST_TEST(bad_token);
	BOOST_TEST(!bad_token->Renew());
	delete bad_token;
}

BOOST_FIXTURE_TEST_CASE(stt_session_test, fixt)
{
	auto token_ = new cYandexGrpcIamToken(config);
	token_->Renew();
	std::string host(config.sttReceiverHostName), t_(token_->GetToken());
	auto session = new cYandexSttSession(host, t_, 8000);
	BOOST_TEST(session);
	std::ifstream f(PROJECT_SOURCE_DIR "/tests/speech.pcm", std::ifstream::binary);
	char buf[16000];
	while (!f.eof() && f.good()) {
		f.read(buf, 16000);
		cYandexSttSession::Sample z{f.gcount(), buf};
		(*session) << z;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		auto alts = session->GetAlternatives();
		if (alts)
			for (const auto &alt: *alts)
				BOOST_TEST_MESSAGE(alt);
	}
	session->Complete();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	{
		auto alts = session->GetAlternatives();
		if (alts)
			for (const auto &res: *alts)
				BOOST_TEST_MESSAGE(res);
	}
	delete session;
	delete token_;
}