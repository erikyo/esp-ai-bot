#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_crt_bundle.h"
#include "audio_codecs.h"
#include "response_handler.h"

#define TAG "HTTP_WHISPER"
#define OPENAI_API_KEY "YOUR_OPENAI_API_KEY"
#define MAX_HTTP_RECV_BUFFER 1024
#define MAX_RESPONSE_TOKEN 1000

char* file_type = "audio/wav";

char* get_file_format(char *file_path) {
    char *dot = strrchr(file_path, '.');
    if (dot == NULL) {
        return NULL;
    }
    return dot + 1;
}

esp_err_t create_whisper_request_from_record(uint8_t *audio, int audio_len)
{
    // Set the authorization headers
    char url[128] = "https://api.openai.com/v1/audio/transcriptions";
    char headers[256];
    snprintf(headers, sizeof(headers), "Bearer %s", OPENAI_API_KEY);
    // Configure the HTTP client
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .event_handler = response_handler,
        .buffer_size = MAX_HTTP_RECV_BUFFER,
        .timeout_ms = 60000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    // Initialize the HTTP client
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Set the headers
    esp_http_client_set_header(client, "Authorization", headers);

    // Set the content type and the boundary string
    char boundary[] = "boundary1234567890";
    char content_type[64];
    snprintf(content_type, sizeof(content_type), "multipart/form-data; boundary=%s", boundary);
    esp_http_client_set_header(client, "Content-Type", content_type);

    // Set the file data and size
    char *file_data = NULL;
    size_t file_size;
    file_data = (char *)audio;
    file_size = audio_len;

    // Build the multipart/form-data request
    char *form_data = (char *)malloc(MAX_HTTP_RECV_BUFFER);
    assert(form_data);
    ESP_LOGI(TAG, "Size of form_data buffer: %zu bytes", sizeof(*form_data) * MAX_HTTP_RECV_BUFFER);
    int form_data_len = 0;
    form_data_len += snprintf(form_data + form_data_len, MAX_HTTP_RECV_BUFFER - form_data_len,
                              "--%s\r\n"
                              "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
                              "Content-Type: application/octet-stream\r\n"
                              "\r\n", boundary, get_file_format(file_type));
    ESP_LOGI(TAG, "form_data_len %d", form_data_len);
    ESP_LOGI(TAG, "form_data %s\n", form_data);

    // Append the audio file contents
    memcpy(form_data + form_data_len, file_data, file_size);
    form_data_len += file_size;
    ESP_LOGI(TAG, "Size of form_data: %zu", form_data_len);

    // Append the rest of the form-data
    form_data_len += snprintf(form_data + form_data_len, MAX_HTTP_RECV_BUFFER - form_data_len,
                              "\r\n"
                              "--%s\r\n"
                              "Content-Disposition: form-data; name=\"model\"\r\n"
                              "\r\n"
                              "whisper-1\r\n"
                              "--%s--\r\n", boundary, boundary);

    // Set the headers and post field
    esp_http_client_set_post_field(client, form_data, form_data_len);

    // Send the request
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "HTTP POST request failed: %s\n", esp_err_to_name(err));
    }

    // Clean up client
    esp_http_client_cleanup(client);

    // Return error code
    return err;
}
