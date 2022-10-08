//
// Created by dbori on 13.08.2022.
//

#ifndef MOD_YANDEX_TRANSCRIBE_YANDEX_IAM_TOKEN_H
#define MOD_YANDEX_TRANSCRIBE_YANDEX_IAM_TOKEN_H

#include <jwt-cpp/jwt.h>
#include <sstream>
#include <switch.h>
#include <switch_curl.h>

#include "yandex/cloud/ai/stt/v3/stt_service.pb.h"
#include "ytg_bindings.h"

using namespace std;

class cYandexGrpcIamToken {
	jwt::builder<jwt::traits::kazuho_picojson> encoded_token;
	string tokenUrl, bearer;
	stringstream tokenStream;
	string jwtReq;
	switch_CURL *curl{nullptr};
	switch_curl_slist_t *list{nullptr};

	static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
	static size_t read_callback(char *buffer, size_t size, size_t nitems, void *userdata);

public:
	explicit cYandexGrpcIamToken(ysg_config_t &config);
	~cYandexGrpcIamToken();
	time_t Renew();
	const char *GetToken();
};

#endif//MOD_YANDEX_TRANSCRIBE_YANDEX_IAM_TOKEN_H
