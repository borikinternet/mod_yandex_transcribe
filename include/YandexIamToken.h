//
// Created by dbori on 13.08.2022.
//

#ifndef MOD_YANDEX_TRANSCRIBE_YANDEXIAMTOKEN_H
#define MOD_YANDEX_TRANSCRIBE_YANDEXIAMTOKEN_H

#include <jwt-cpp/jwt.h>
#include <sstream>
#include <switch.h>
#include <switch_curl.h>
#include <shared_mutex>
#include <fstream>

#include "ytg_bindings.h"

using namespace std;

class cYandexGrpcIamToken {
	jwt::builder<jwt::traits::kazuho_picojson> encoded_token;
	string tokenUrl, bearer, jwtReq;
	unique_ptr<stringstream> tokenStream{nullptr};
	switch_CURL *curl{nullptr};
	switch_curl_slist_t *list{nullptr};
	std::shared_mutex l_;
	std::ifstream priv_key_file, pub_key_file;
	jwt::algorithm::ps256 algorithm;

	static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
	static size_t read_callback(char *buffer, size_t size, size_t nitems, void *userdata);

public:
	explicit cYandexGrpcIamToken(const ysg_config_t &config);
	~cYandexGrpcIamToken();
	time_t Renew();
	string GetToken();
};

#endif//MOD_YANDEX_TRANSCRIBE_YANDEXIAMTOKEN_H
