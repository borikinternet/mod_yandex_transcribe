//
// Created by dbori on 08.10.2022.
//
#define BOOST_TEST_MODULE iamtoken_tests

#define TEST 1

#include <boost/test/unit_test.hpp>

#include "yandex_iam_token.h"

BOOST_AUTO_TEST_CASE(connection_test) {
	ysg_config_t config{"aje95nign4v8tj6cl49f", "ajenj092br8692njemn0", "../conf/speechkit_id.pub",
	                    "../conf/speechkit_id", "https://iam.api.cloud.yandex.net/iam/v1/tokens"};
	auto token = new cYandexGrpcIamToken(config);
	BOOST_TEST(token);
	BOOST_TEST(token->Renew());
	BOOST_TEST(strlen(token->GetToken()));
	delete token;
	auto incorrect_config = config;
	incorrect_config.pubKeyId = "zzz";
	auto bad_token = new cYandexGrpcIamToken(incorrect_config);
	BOOST_TEST(bad_token);
	BOOST_TEST(!bad_token->Renew());
	delete bad_token;
}