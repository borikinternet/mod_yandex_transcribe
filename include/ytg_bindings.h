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
} ysg_config_t;

switch_status_t init_yandex_grpc(ysg_config_t *config);
time_t get_iam_token();
switch_status_t destroy_yandex_grpc();

#ifdef __cplusplus
}
#endif

#endif//MOD_YANDEX_TRANSCRIBE_YTG_BINDINGS_H
