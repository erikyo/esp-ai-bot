#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_crt_bundle.h"
#include "audio_codecs.h"
#include "response_handler.h"

#define VOICE_ID "your_voice_id"
#define VOLUME 5
#define MAX_FILE_SIZE (20 * 1024)

static const char* TAG = "TEXT_TO_SPEECH";

esp_err_t text_to_speech_request(const char *message, AUDIO_CODECS_FORMAT code_format)
{
    size_t message_len = strlen(message);
    char *encoded_message;
    char *codec_format_str;

    // Encode the message for URL transmission
    encoded_message = heap_caps_malloc((3 * message_len + 1), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    url_encode(message, encoded_message);

    // Determine the audio codec format
    if (AUDIO_CODECS_MP3 == code_format) {
        codec_format_str = "MP3";
    } else {
        codec_format_str = "WAV";
    }

    // Determine the required size of the URL bu
    int url_size = snprintf(NULL, 0, "https://dds.dui.ai/runtime/v1/synthesize?voiceId=%s&text=%s&speed=1&volume=%d&audiotype=%s", \
                            VOICE_ID, \
                            encoded_message, \
                            VOLUME, \
                            codec_format_str);

    // Allocate memory for the URL buffer
    char *url = heap_caps_malloc((url_size + 1), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (url == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for URL");
        return ESP_ERR_NO_MEM;
    }

    // Format the URL string
    snprintf(url, url_size + 1, "https://dds.dui.ai/runtime/v1/synthesize?voiceId=%s&text=%s&speed=1&volume=%d&audiotype=%s", \
             VOICE_ID, \
             encoded_message, \
             VOLUME, \
             codec_format_str);

    // Configure the HTTP client
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .event_handler = http_event_handler,
        .buffer_size = MAX_FILE_SIZE,
        .buffer_size_tx = 4000,
        .timeout_ms = 30000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    // Initialize and perform the HTTP request
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    // Free allocated memory and clean up the HT
    heap_caps_free(url);
    heap_caps_free(encoded_message);
    esp_http_client_cleanup(client);

    // Return the result of the function call
    return err;
}