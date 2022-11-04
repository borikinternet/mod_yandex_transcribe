//
// Created by dbori on 08.10.2022.
//
#define BOOST_TEST_MODULE iamtoken_tests

#define TEST 1

#include "tests/config.h"
#include <boost/test/unit_test.hpp>

#include "YandexIamToken.h"
#include "YandexSttSession.h"

struct fixt {
	ysg_config_t config{"aje95nign4v8tj6cl49f", "ajenj092br8692njemn0",
	                    PROJECT_SOURCE_DIR "/conf/speechkit_id.pub",
	                    PROJECT_SOURCE_DIR "/conf/speechkit_id",
	                    "https://iam.api.cloud.yandex.net/iam/v1/tokens"};
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

}