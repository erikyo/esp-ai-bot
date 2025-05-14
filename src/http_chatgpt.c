#include <stdio.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "audio_codecs.h"
#include "response_handler.h"

#define TAG "HTTP_CHATGPT"
#define OPENAI_API_KEY "YOUR_OPENAI_API_KEY"
#define MAX_HTTP_RECV_BUFFER 1024
#define MAX_RESPONSE_TOKEN 1000
#define json_fmt "{\"model\": \"%s\", \"max_tokens\": %d, \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}]}"

static char json_payload[1024];
static char response_buffer[MAX_HTTP_RECV_BUFFER];
static int response_len = 0;

esp_err_t create_chatgpt_request(const char *content)
{
    char url[128] = "https://api.openai.com/v1/chat/completions";
    char model[16] = "gpt-3.5-turbo";
    char headers[256];
    snprintf(headers, sizeof(headers), "Bearer %s", OPENAI_API_KEY);

    // Reset the response buffer
    memset(response_buffer, 0, MAX_HTTP_RECV_BUFFER);
    response_len = 0;

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .event_handler = response_handler,
        .buffer_size = MAX_HTTP_RECV_BUFFER,
        .timeout_ms = 30000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    // Set the headers
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Authorization", headers);

    // Create JSON payload with model, max tokens, and content
    snprintf(json_payload, sizeof(json_payload), json_fmt, model, MAX_RESPONSE_TOKEN, content);
    esp_http_client_set_post_field(client, json_payload, strlen(json_payload));

    // Send the request
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "HTTP POST request failed: %s\n", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Risposta: %s", response_buffer);
    }

    // Clean up client
    esp_http_client_cleanup(client);

    // Return error code
    return err;
}