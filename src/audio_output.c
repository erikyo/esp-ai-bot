#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

extern const int I2S_PORT_SPKR;
extern const int I2S_PORT_MIC;

#define I2S_PORT_SPKR       I2S_NUM_0
#define I2S_BCK_IO          22
#define I2S_WS_IO           21
#define I2S_DOUT_IO         19

static const int I2S_BITS_PER_SAMPLE = 16;
static const int I2S_SAMPLE_RATE = 16000;

void audio_output_init(void)
{
    // Create new I2S channel
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_PORT_SPKR, I2S_ROLE_MASTER);
    i2s_new_channel(&chan_cfg, I2S_PORT_SPKR, NULL);

    // Configure the standard I2S controller
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_BITS_PER_SAMPLE, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK_IO,
            .ws = I2S_WS_IO,
            .dout = I2S_DOUT_IO,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    // Initialize the standard I2S controller
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(I2S_PORT_SPKR, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(I2S_PORT_SPKR));

    ESP_LOGI("I2S", "🔊 MAX98357A initialized.");
}

void audio_player_play(const char *audio_data, int len)
{
    ESP_LOGI("PLAYER", "▶️ playing audio (%d bytes)", len);

    size_t bytes_written = 0;
    int offset = 0;

    while (offset < len) {
        int chunk = (len - offset > 512) ? 512 : len - offset;
        i2s_channel_write(I2S_PORT_SPKR, audio_data + offset, chunk, &bytes_written, portMAX_DELAY);
        offset += bytes_written;
    }

    ESP_LOGI("PLAYER", "✅ Audio played successfully.");
}