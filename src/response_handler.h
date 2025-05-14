#include <esp_http_client.h>

esp_err_t response_handler(esp_http_client_event_t *evt);
void url_encode(const char *src, char *dest);
esp_err_t http_event_handler(esp_http_client_event_t *evt);