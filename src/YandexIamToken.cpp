//
// Created by dbori on 13.08.2022.
//

#include "YandexIamToken.h"
#include <chrono>
#include <date/date.h>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>

static cYandexGrpcIamToken *token = nullptr;

#ifdef __cplusplus
extern "C" {
#endif
/**
 *  Get new IAM token form Yandex.cloud
 * @return how many seconds new token will be active or 0 for unsuccessful renew
 */
time_t get_iam_token() {
	if (!token)
		return 0;
	return token->Renew();
}

switch_status_t init_yandex_grpc(ysg_config_t *config) {
	try {
		if (!token)
			token = new cYandexGrpcIamToken(*config);
	} catch (exception &e) {
		return SWITCH_STATUS_NOT_INITALIZED;
	}
	return SWITCH_STATUS_SUCCESS;
}

switch_status_t destroy_yandex_grpc() {
	delete token;
	return SWITCH_STATUS_SUCCESS;
}

#ifdef __cplusplus
}
#endif

size_t cYandexGrpcIamToken::read_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
	static size_t written = 0;
	auto self = reinterpret_cast<cYandexGrpcIamToken *>(userdata);
	auto str = self->jwtReq.data();

	str += written;
	auto len = strlen(str);
	if (len) {
		written += min(size * nitems, len);
		strncpy(buffer, str, size * nitems);
	} else
		written = 0;
	return len;
}

cYandexGrpcIamToken::cYandexGrpcIamToken(const ysg_config_t &config)
	: tokenUrl(config.getTokenUrl) {
	std::ifstream priv_key_file(config.privKeyFile);
	std::ifstream pub_key_file(config.pubKeyFile);

	// todo fix exception class
	if (!priv_key_file.good() || !pub_key_file.good())
		throw exception();

	auto now = std::chrono::system_clock::now();
	auto expires_at = now + std::chrono::hours(1);
	auto serviceAccountId = config.accId;
	auto keyId = config.pubKeyId;
	auto algorithm = jwt::algorithm::ps256(
		std::string(std::istreambuf_iterator<char>{pub_key_file}, {}),
		std::string(std::istreambuf_iterator<char>{priv_key_file}, {}));

	// Формирование JWT.
	encoded_token = jwt::create();
	encoded_token.set_key_id(keyId);
	encoded_token.set_issuer(serviceAccountId);
	encoded_token.set_audience(tokenUrl);
	encoded_token.set_issued_at(now);
	encoded_token.set_expires_at(expires_at);

	jwtReq = R"({"jwt": ")" + encoded_token.sign(algorithm) + R"("})";

	curl = switch_curl_easy_init();
	if (!curl)
		throw exception();
	auto retCode = switch_curl_easy_setopt(curl, CURLOPT_URL, tokenUrl.data());
	if (retCode != CURLE_OK)
		throw exception();
	switch_curl_easy_setopt(curl, CURLOPT_POST, 1L);
	switch_curl_easy_setopt(curl, CURLOPT_POSTFIELDS, nullptr);

	list = switch_curl_slist_append(list, "Content-Type: application/json");
	switch_curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	switch_curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jwtReq.length());
	switch_curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
	switch_curl_easy_setopt(curl, CURLOPT_READDATA, this);
	switch_curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	switch_curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
}

time_t cYandexGrpcIamToken::Renew() {
	if (!curl)
		return 0;
	tokenStream.clear();
	auto retCode = switch_curl_easy_perform(curl);
	if (retCode) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
		                  "Error while trying to update token, HTTP code %i", retCode);
		return 0;
	}
	auto j = nlohmann::json::parse(tokenStream.str());
	if (!j.contains("iamToken") || !j.contains("expiresAt")) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
		                  "Error while trying to update token, Yandex reply: %s", tokenStream.str().data());
		return 0;
	}
	bearer = j["iamToken"].get<string>();
	stringstream expiresAtStr;
	expiresAtStr << j["expiresAt"].get<string>();
	date::sys_seconds tps;
	expiresAtStr >> date::parse("%FT%T%Ez", tps);
	auto expires = std::chrono::duration_cast<std::chrono::microseconds>(tps - std::chrono::system_clock::now());
	return expires.count();
}

cYandexGrpcIamToken::~cYandexGrpcIamToken() {
	if (curl)
		switch_curl_easy_cleanup(curl);
	if (list)
		switch_curl_slist_free_all(list);
}

size_t cYandexGrpcIamToken::write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	auto self = reinterpret_cast<cYandexGrpcIamToken *>(userdata);
	char buf[CURL_MAX_WRITE_SIZE + 1];
	char *str = buf;
	buf[size * nmemb] = '\0';
	memcpy(buf, ptr, size * nmemb);
	self->tokenStream << str;
	return nmemb;
}

string &cYandexGrpcIamToken::GetToken() {
	return bearer;
}
