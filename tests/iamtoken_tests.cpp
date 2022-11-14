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

struct fixt {
	ysg_config_t config{"aje95nign4v8tj6cl49f", "ajenj092br8692njemn0",
	                    PROJECT_SOURCE_DIR "/conf/speechkit_id.pub",
	                    PROJECT_SOURCE_DIR "/conf/speechkit_id",
	                    "https://iam.api.cloud.yandex.net/iam/v1/tokens",
	                    "stt.api.cloud.yandex.net:443"};
};

BOOST_FIXTURE_TEST_CASE(connection_test, fixt)
{
	BOOST_TEST_MESSAGE(config.privKeyFile);
	BOOST_TEST_MESSAGE(config.pubKeyFile);
	auto token = new cYandexGrpcIamToken(config);
	BOOST_TEST(token);
	BOOST_TEST(token->Renew());
	BOOST_TEST(token->GetToken().length());
	delete token;
	const ysg_config_t incorrect_config({config.accId, "zzz", config.pubKeyFile,
	                                     config.privKeyFile, config.getTokenUrl});
	auto bad_token = new cYandexGrpcIamToken(incorrect_config);
	BOOST_TEST(bad_token);
	BOOST_TEST(!bad_token->Renew());
	delete bad_token;
}

BOOST_FIXTURE_TEST_CASE(stt_session_test, fixt)
{
	auto token = new cYandexGrpcIamToken(config);
	token->Renew();
	auto session = new cYandexSttSession({config.sttReceiverHostName}, token->GetToken(), "some-uuid", 8000);
	BOOST_TEST(session);
	std::ifstream f(PROJECT_SOURCE_DIR "/tests/speech.pcm", std::ifstream::binary);
	char buf[16000];
	while (!f.eof() && f.good()) {
		f.read(buf, 16000);
		cYandexSttSession::Sample z{f.gcount(), buf};
		(*session) << z;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		auto res = session->GetLastResponse();
		BOOST_TEST_MESSAGE(res.DebugString());
	}
	session->Complete();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	auto resp = session->GetLastResponse();
	BOOST_TEST_MESSAGE(resp.DebugString());
	for (const auto& res : session->GetAlternatives())
		BOOST_TEST_MESSAGE(res);
	delete session;
	delete token;
}