#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"

#include "audio_codecs.h"
#include "esp_log.h"
#include "esp_system.h"
#include "wifi_connect.h"
#include "audio_input.h"
#include "audio_output.h"
#include "http_chatgpt.h"
#include "http_whisper.h"
#include "text_to_speech.h"

#define MAX_FILE_SIZE 10000
#define TAG "APP_MAIN"

char record_audio_buffer[MAX_FILE_SIZE];
char message_content[1024];

#define I2S_BITS_PER_SAMPLE_16 16

void app_main(void)
{
    // Initialize audio input and output
    i2s_init();              // INMP441
    audio_output_init();     // MAX98357A
    wifi_connect();          // connect to wifi

    while (1) {
        ESP_LOGI(TAG, "🟢 Recording");

        // 1. get the audio 
        int audio_len = record_audio(record_audio_buffer, MAX_FILE_SIZE);
        if (audio_len <= 0) {
            ESP_LOGE(TAG, "❌ Error recording audio");
            continue;
        }

        // 2. send the audio to Whisper
        if (create_whisper_request_from_record((uint8_t *)record_audio_buffer, audio_len) != ESP_OK) {
            ESP_LOGE(TAG, "❌ Error sending audio to Whisper");
            continue;
        }

        // 3. send the message to ChatGPT
        if (create_chatgpt_request(message_content) != ESP_OK) {
            ESP_LOGE(TAG, "❌ Error sending message to ChatGPT");
            continue;
        }

        // 4. text to speech
        if (text_to_speech_request(message_content, AUDIO_CODECS_MP3) != ESP_OK) {
            ESP_LOGE(TAG, "❌ Error converting text to speech");
            continue;
        }

        ESP_LOGI(TAG, "✅ Loop completed wait 5 seconds...");
        vTaskDelay(pdMS_TO_TICKS(5000)); 
    }
}
