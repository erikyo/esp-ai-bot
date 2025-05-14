#include <stdio.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_crt_bundle.h"
#include "audio_codecs.h"

#define TAG "HTTP_CHATGPT"
#define MAX_HTTP_RECV_BUFFER 1024

static char response_buffer[MAX_HTTP_RECV_BUFFER];
static int response_len = 0;

// Modifica: cambiare il tipo di ritorno da char* a esp_err_t
esp_err_t response_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // Gestione dei dati ricevuti
            if (response_len + evt->data_len < MAX_HTTP_RECV_BUFFER) {
                memcpy(response_buffer + response_len, evt->data, evt->data_len);
                response_len += evt->data_len;
                response_buffer[response_len] = 0;
            } else {
                ESP_LOGW(TAG, "Buffer di risposta troppo piccolo");
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "Risposta ricevuta");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Connessione chiusa");
            break;
        default:
            break;
    }
    return ESP_OK; // Ritorna ESP_OK per indicare successo
}

void url_encode(const char *src, char *dest)
{
    static const char *hex = "0123456789ABCDEF";
    while (*src) {
        if (('a' <= *src && *src <= 'z') ||
            ('A' <= *src && *src <= 'Z') ||
            ('0' <= *src && *src <= '9')) {
            *dest++ = *src;
        } else {
            *dest++ = '%';
            *dest++ = hex[(*src >> 4) & 0xF];
            *dest++ = hex[*src & 0xF];
        }
        src++;
    }
    *dest = '\0';
}


esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // gestisci i dati ricevuti, es. scrivili su un buffer
            break;
        default:
            break;
    }
    return ESP_OK;
}