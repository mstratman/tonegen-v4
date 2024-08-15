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

#include "pico/stdlib.h"
#include "pico/audio_i2s.h"  // pico-extras
#include <math.h>

#include "constants.h"
#include "debug.h"

#define PLUCK_TIME 70 // each note will play at least this long (ms)
#define NOTE_CHUNK_TIME 10 // we damp in intervals of this (ms)
#define MAX_NOTE_TIME 3000 // how much time beyond PLUCK_TIME might it ring out (ms)
#define RAMP_AMOUNT 0.01f

#define TONE_VOL 0.4f // to match samples

#define NOTE_LENGTH_PERCENT_OF_SPEED 0.8f // 80% of the scaled_speed will be filled with tone
uint16_t speed; // 0-MAX_POT. Proportion of MAX_NOTE_TIME for repeating notes

// cache the calculated values, (ms)
uint16_t scaled_speed;
uint16_t scaled_length;
float damp_amount = 0.01; // arbitrary starting value. Really you need to call set_tone_speed() first

#define SINE_WAVE_TABLE_LEN 2048
static int16_t sine_wave_table[SINE_WAVE_TABLE_LEN];

#define NUM_TONES 8 // see play_tone()
uint8_t tone_i = 0;

uint16_t freq;
float phase = 0.0f; // phase accumulator
float delta_phi;

struct audio_buffer_pool *tp_ap;

void set_tone_speed(uint16_t potval) {
    speed = potval;
    scaled_speed = (uint16_t)(((float)speed/(float)MAX_POT)*MAX_NOTE_TIME);
    scaled_length = (uint16_t)((float)NOTE_LENGTH_PERCENT_OF_SPEED*scaled_speed);
    damp_amount = TONE_VOL/(SAMPLE_RATE_MS*scaled_length);
}

void _set_freq(uint16_t f) {
  freq = f;
  delta_phi = (float) ((float)freq / (float)SAMPLE_RATE) * (float)SINE_WAVE_TABLE_LEN;
}


// NOTE: This blocks
void _pluck(uint16_t f) {
    _set_freq(f);

    float vol = 0.0f;

    // initial pluck causes undampened tone for PLUCK_TIME
    // Also ramp up volume to avoid clicks and pops
    absolute_time_t pluck_stop = make_timeout_time_ms((uint32_t)PLUCK_TIME);
    while (! time_reached(pluck_stop)) {
        struct audio_buffer *buffer = take_audio_buffer(tp_ap, true);
        int16_t *samples = (int16_t *)buffer->buffer->bytes;
        for (uint i = 0; i < buffer->max_sample_count; i++) {
            if (vol < TONE_VOL) {
                vol += RAMP_AMOUNT;
            }
            samples[i] = (int16_t)(vol * sine_wave_table[(int)phase]);
            phase += delta_phi;
            if (phase >= (float)SINE_WAVE_TABLE_LEN) {
                phase -= (float)SINE_WAVE_TABLE_LEN;
            }
        }
        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(tp_ap, buffer);
    }
    
    // play a dampening sound
    //pluck_stop = make_timeout_time_ms((uint32_t)scaled_length);
    //while (! time_reached(pluck_stop)) {
    while (vol > 0) {
        struct audio_buffer *buffer = take_audio_buffer(tp_ap, true);
        int16_t *samples = (int16_t *)buffer->buffer->bytes;
        for (uint i = 0; i < buffer->max_sample_count; i++) {
            samples[i] = (int16_t)(vol * sine_wave_table[(int)phase]);

            vol -= damp_amount;

            phase += delta_phi;
            if (phase >= (float)SINE_WAVE_TABLE_LEN) {
                phase -= (float)SINE_WAVE_TABLE_LEN;
            }
        }
        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(tp_ap, buffer);
    }


    // play silence
    if (scaled_speed > scaled_length) {
        /*
        pluck_stop = make_timeout_time_ms((uint32_t)scaled_length);
        while (!time_reached(pluck_stop)) {
            struct audio_buffer *buffer = take_audio_buffer(tp_ap, true);
            int16_t *samples = (int16_t *)buffer->buffer->bytes;
            for (uint i = 0; i < buffer->max_sample_count; i++) {
                samples[i] = 0;
            }
            buffer->sample_count = buffer->max_sample_count;
            give_audio_buffer(tp_ap, buffer);
        }
        */
       busy_wait_ms(scaled_speed - scaled_length);
    }

}
void _queue_freq(uint16_t f) {
    _set_freq(f);
    struct audio_buffer *buffer = take_audio_buffer(tp_ap, true);
    int16_t *samples = (int16_t *)buffer->buffer->bytes;
    for (uint i = 0; i < buffer->max_sample_count; i++) {
        samples[i] = (int16_t)(TONE_VOL * sine_wave_table[(int)phase]);
        phase += delta_phi;
        if (phase >= (float)SINE_WAVE_TABLE_LEN) {
            phase -= (float)SINE_WAVE_TABLE_LEN;
        }
    }
    buffer->sample_count = buffer->max_sample_count;
    give_audio_buffer(tp_ap, buffer);
}




void next_tone() {
    tone_i++;
    if (tone_i >= NUM_TONES) {
        tone_i = 0;
    }
    PF("tone_i=%d\n", tone_i);
}
void set_tone_num(uint8_t i) {
    tone_i = i;
}
uint8_t get_tone_num() {
    return tone_i;
}




void tone_init() {
    for (int i = 0; i < SINE_WAVE_TABLE_LEN; i++) {
        sine_wave_table[i] = 32767 * cosf(i * 2 * (float)(M_PI / SINE_WAVE_TABLE_LEN));
    }
    _set_freq(440);
}

// entry point, main function. Can block for one "plucked" note.
void play_tone(struct audio_buffer_pool *ap) {
    tp_ap = ap;
    // continuous tones
    if (tone_i == 0) {
        _queue_freq(262);
    } else if (tone_i == 1) {
        _pluck(262);

    } else if (tone_i == 2) {
        _queue_freq(392);
    } else if (tone_i == 3) {
        _pluck(392);

    } else if (tone_i == 4) {
        _queue_freq(523);
    } else if (tone_i == 5) {
        _pluck(523);

    } else if (tone_i == 6) {
        _queue_freq(1047);
    } else if (tone_i == 7) {
        _pluck(1047);
    }
}


