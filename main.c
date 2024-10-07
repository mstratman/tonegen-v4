/*
Copyright (C) 2024  Mark A. Stratman <mark@mas-effects.com>

This file is part of tonegen-v4

tonegen-v4 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

tonegen-v4 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with tonegen-v4; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include "hardware/adc.h"
#include "pico/audio_i2s.h"  // pico-extras
#include "pico/stdlib.h"

#include "button.h"

#include "constants.h"
#include "debug.h"
#include "sample_player.h"
#include "tone_player.h"
#include "flash_settings.h"

#define PIN_LED_TONE 16
#define PIN_BUTTON_TONE 17
#define PIN_LED_SAMPLE 19
#define PIN_BUTTON_SAMPLE 18

#define POT_DEBOUNCE 4
#define PIN_POT 26  // this is ADC0 == PIN 26
#define POT_ADC 0   // this is ADC0 == PIN 26
// see also the pin definitions in the CMakeLists.txt for I2S

// After user presses a button, wait this
// long and then save the settings.
#define SAVE_SETTINGS_AFTER_MS 4000
alarm_id_t settings_alarm_id;

uint16_t last_pot_val = 0;

#define MODE_SAMPLE 0
#define MODE_TONE 1
uint8_t mode = MODE_SAMPLE;

#define SAMPLES_PER_BUFFER 256 // TODO: What should this be?

struct audio_buffer_pool *ap;


struct audio_buffer_pool *init_audio() {
    // TODO: Update this
    static audio_format_t audio_format = {
        .format = AUDIO_BUFFER_FORMAT_PCM_S16,
        .sample_freq = SAMPLE_RATE,  
        .channel_count = 1,
    };
    static struct audio_buffer_format producer_format = {
        .format = &audio_format,
        .sample_stride = 2  // TODO what is this? buffer size = sample count *
                            // stride. audio_i2s.c asserts it must be 2.
    };

    // NOTE: this calloc()'s
    struct audio_buffer_pool *producer_pool =
        audio_new_producer_pool(&producer_format,
                                3,  // buffer count? what is this
                                SAMPLES_PER_BUFFER);  // TODO: correct size
    // bool __unused ok;
    const struct audio_format *output_format;
    struct audio_i2s_config config = {
        .data_pin = PICO_AUDIO_I2S_DATA_PIN,
        .clock_pin_base = PICO_AUDIO_I2S_CLOCK_PIN_BASE,
        .dma_channel = 0,
        .pio_sm = 0,
    };
    output_format = audio_i2s_setup(&audio_format, &config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }
    // ok = audio_i2s_connect(producer_pool);
    if (audio_i2s_connect(producer_pool)) {
        P("audio_i2s_connect returned true\n");
    } else {
        P("ERROR ERROR ERROR: audio_i2s_connect failed\n");
    }
    // assert(ok);
    audio_i2s_set_enabled(true);
    return producer_pool;
}

int64_t settings_save_callback(alarm_id_t id, void *user_data) {
    settings_t s = {mode, get_sample_num(), get_tone_num() };
    flash_update_settings(s);
    settings_alarm_id = 0;
    return 0;
}

void on_button_change(button_t *button_p) {
    button_t *button = (button_t *)button_p;
    PF("Button on pin %d changed its state to %d\n", button->pin, button->state);

    if (button->state)
        return;  // Ignore button release.

    switch (button->pin) {
        case PIN_BUTTON_SAMPLE:
            gpio_put(PIN_LED_TONE, 0);
            gpio_put(PIN_LED_SAMPLE, 1);
            P("Sample\n");
            if (mode == MODE_SAMPLE) {
              next_sample();
            } else {
                mode = MODE_SAMPLE;
            }
            break;
        case PIN_BUTTON_TONE:
            P("SCALE\n");
            gpio_put(PIN_LED_TONE, 1);
            gpio_put(PIN_LED_SAMPLE, 0);
            if (mode == MODE_TONE) {
                next_tone();
            } else {
                mode = MODE_TONE;
            }
            break;
    }

    if (settings_alarm_id > 0) {
        if (!cancel_alarm(settings_alarm_id)) {
            P("****** FAILED to cancel alarm*****\n");
        }
    }
    settings_alarm_id = add_alarm_in_ms(SAVE_SETTINGS_AFTER_MS, settings_save_callback, NULL, true);
    if (settings_alarm_id <= 0) {
        PF("***FAILED TO ADD ALARM to save settings id=%d******\n", settings_alarm_id);
    }
}


void set_tone_speed_from_pot() {
    uint16_t val = adc_read();
    if (abs(val - last_pot_val) <= POT_DEBOUNCE) {
        return;
    }

    last_pot_val = val;

    set_tone_speed(val);
}

int main() {
    stdio_init_all();

#ifndef NDEBUG
    timer_hw->dbgpause = 0;
#endif

    adc_init();
    adc_gpio_init(PIN_POT);
    adc_select_input(POT_ADC);

    gpio_init(PIN_LED_SAMPLE);
    gpio_set_dir(PIN_LED_SAMPLE, GPIO_OUT);
    gpio_init(PIN_LED_TONE);
    gpio_set_dir(PIN_LED_TONE, GPIO_OUT);

    gpio_put(PIN_LED_SAMPLE, 1);  // TODO: if this is read from flash, do it conditionally

    ap = init_audio();

    // these are malloc'ed
    button_t *scale_button = create_button(PIN_BUTTON_TONE, on_button_change);
    button_t *sample_button = create_button(PIN_BUTTON_SAMPLE, on_button_change);

    tone_init();
    set_tone_speed_from_pot();

    settings_t settings = flash_read_settings();
    set_tone_num(settings.tone_num);
    set_sample_num(settings.sample_num);
    mode = settings.mode;
    if (mode == MODE_SAMPLE) {
        gpio_put(PIN_LED_TONE, 0);
        gpio_put(PIN_LED_SAMPLE, 1);
    } else {
        gpio_put(PIN_LED_TONE, 1);
        gpio_put(PIN_LED_SAMPLE, 0);
    }
    
    PF("PICO_FLASH_SIZE_BYTES=%d", PICO_FLASH_SIZE_BYTES);

    while (true) {
        if (mode == MODE_SAMPLE) {
            play_sample(ap);
        } else {
            set_tone_speed_from_pot();
            play_tone(ap);
        }
    }

    return 0;
}
