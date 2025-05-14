#include "driver/i2s.h"
#include "esp_log.h"

extern const int I2S_SAMPLE_RATE;
extern const int I2S_PORT_MIC;

static const int I2S_BITS_PER_SAMPLE_16 = 16;

#define I2S_SAMPLE_RATE     16000
#define I2S_CHANNEL_NUM     1
#define I2S_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16
#define I2S_PORT_MIC        I2S_NUM_0

/**
 * @brief Initializes the I2S driver for microphone input.
 * 
 * This function configures the I2S peripheral for receiving audio data from a microphone.
 * It sets up the I2S configuration for master mode with RX capabilities and initializes
 * the DMA buffers. The pin configuration is set to match the connected microphone hardware.
 * 
 * The function installs the I2S driver and assigns the specified pins to the I2S peripheral.
 * It also clears the DMA buffer to ensure no residual data is present. Once initialized,
 * the I2S driver is ready to capture audio data from the microphone.
 */
void i2s_init(void)
{
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,  // Solo ricezione (microfono)
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 6,
        .dma_buf_len = 512,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = 26,
        .ws_io_num = 25,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = 34 // Collegato a INMP441
    };

    i2s_driver_install(I2S_PORT_MIC, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT_MIC, &pin_config);
    i2s_zero_dma_buffer(I2S_PORT_MIC);

    ESP_LOGI("I2S", "Microfono INMP441 inizializzato.");
}

int record_audio(char *buffer, int max_len)
{
    size_t bytes_read = 0;
    int total_read = 0;

    ESP_LOGI("AUDIO", "🎙️ Inizio registrazione...");

    while (total_read < max_len) {
        i2s_read(I2S_PORT_MIC, buffer + total_read, 512, &bytes_read, portMAX_DELAY);
        total_read += bytes_read;

        if (bytes_read <= 0) {
            break;
        }
    }

    ESP_LOGI("AUDIO", "✅ Registrazione completata (%d bytes)", total_read);
    return total_read;
}
