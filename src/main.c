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
    // Inizializzazione periferiche
    i2s_init();              // INMP441
    audio_output_init();     // MAX98357A
    wifi_connect();          // Assicurati che sia connesso al WiFi

    while (1) {
        ESP_LOGI(TAG, "🟢 Inizio ciclo di interazione");

        // 1. Acquisisci audio (registrazione da microfono)
        int audio_len = record_audio(record_audio_buffer, MAX_FILE_SIZE);
        if (audio_len <= 0) {
            ESP_LOGE(TAG, "❌ Errore nella registrazione audio");
            continue;
        }

        // 2. Invia audio a Whisper per ottenere testo
        if (create_whisper_request_from_record((uint8_t *)record_audio_buffer, audio_len) != ESP_OK) {
            ESP_LOGE(TAG, "❌ Errore nella trascrizione Whisper");
            continue;
        }

        // A questo punto `message_content` contiene il testo trascritto

        // 3. Invia il testo a ChatGPT
        if (create_chatgpt_request(message_content) != ESP_OK) {
            ESP_LOGE(TAG, "❌ Errore nella richiesta ChatGPT");
            continue;
        }

        // Ora `message_content` contiene la risposta di ChatGPT

        // 4. Converti il testo in voce
        if (text_to_speech_request(message_content, AUDIO_CODECS_MP3) != ESP_OK) {
            ESP_LOGE(TAG, "❌ Errore nella sintesi vocale");
            continue;
        }

        ESP_LOGI(TAG, "✅ Ciclo completato, attesa prima di nuovo ciclo...");
        vTaskDelay(pdMS_TO_TICKS(5000)); // Attendi 5 secondi prima della prossima interazione
    }
}
