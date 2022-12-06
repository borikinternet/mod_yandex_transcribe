//
// Created by dbori on 15.08.2022.
//

#ifndef MOD_YANDEX_TRANSCRIBE_YTG_BINDINGS_H
#define MOD_YANDEX_TRANSCRIBE_YTG_BINDINGS_H

#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char *accId;
	const char *pubKeyId;
	const char *pubKeyFile;
	const char *privKeyFile;
	const char *getTokenUrl;
	const char *sttReceiverHostName;
	const char *preferredLanguage;
} ysg_config_t;

switch_status_t init_yandex_grpc(ysg_config_t *config, void **pToken);
time_t renew_iam_token(void *pToken);
switch_status_t destroy_yandex_grpc(void **pToken);
void *create_yandex_stt_session(ysg_config_t *config, unsigned sample_rate, void *pToken);
switch_status_t check_yandex_stt_results(void *session);
switch_status_t get_yandex_stt_results(void *session, char **results, switch_memory_pool_t *pool);
switch_status_t destroy_yandex_stt_session(void **session);
switch_status_t feed_to_yandex_stt_session(void *session, void *data, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif//MOD_YANDEX_TRANSCRIBE_YTG_BINDINGS_H
