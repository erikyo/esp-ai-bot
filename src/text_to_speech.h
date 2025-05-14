#include "esp_http_client.h"

void url_encode(const char *src, char *dest);
esp_err_t text_to_speech_request(const char *message, AUDIO_CODECS_FORMAT code_format);