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

#include "samples/sample01-s16bit-16k.h"
#include "samples/sample02-s16bit-16k.h"
#include "samples/sample03-s16bit-16k.h"
#include "samples/sample04-s16bit-16k.h"
#include "samples/sample05-s16bit-16k.h"
#include "samples/sample06-s16bit-16k.h"
#include "samples/sample07-s16bit-16k.h"
#include "samples/sample08-s16bit-16k.h"
#include "samples/sample09-s16bit-16k.h"
#include "samples/sample10-s16bit-16k.h"
#include "samples/sample11-s16bit-16k.h"
#include "samples/sample12-s16bit-16k.h"
#include "samples/sample13-s16bit-16k.h"
#include "samples/sample14-s16bit-16k.h"
#include "samples/sample15-s16bit-16k.h"
#include "samples/sample16-s16bit-16k.h"
#include "samples/sample17-s16bit-16k.h"
#include "samples/sample18-s16bit-16k.h"
#include "samples/sample19-s16bit-16k.h"
#include "samples/sample20-s16bit-16k.h"
#include "samples/sample21-s16bit-16k.h"
#include "samples/sample22-s16bit-16k.h"
#include "samples/sample23-s16bit-16k.h"
#include "samples/sample24-s16bit-16k.h"
#include "samples/sample25-s16bit-16k.h"
#define NUM_RECORDINGS 25


uint8_t recording_i = 0;
const int16_t *recordings[NUM_RECORDINGS] = {
    sample16,
    sample11,
    sample14,
    sample6,
    sample4,
    sample19,
    sample1,
    sample2,
    sample3,
    sample20,
    sample21,
    sample7,
    sample5,
    sample18,
    sample17,
    sample13,
    sample12,
    sample23,
    sample22,
    sample10,
    sample9,
    sample8,
    sample15,
    sample24,
    sample25
};
uint32_t recording_num_samples[NUM_RECORDINGS] = {
    NUM_SAMPLE16_ELEMENTS,
    NUM_SAMPLE11_ELEMENTS,
    NUM_SAMPLE14_ELEMENTS,
    NUM_SAMPLE6_ELEMENTS,
    NUM_SAMPLE4_ELEMENTS,
    NUM_SAMPLE19_ELEMENTS,
    NUM_SAMPLE1_ELEMENTS,
    NUM_SAMPLE2_ELEMENTS,
    NUM_SAMPLE3_ELEMENTS,
    NUM_SAMPLE20_ELEMENTS,
    NUM_SAMPLE21_ELEMENTS,
    NUM_SAMPLE7_ELEMENTS,
    NUM_SAMPLE5_ELEMENTS,
    NUM_SAMPLE18_ELEMENTS,
    NUM_SAMPLE17_ELEMENTS,
    NUM_SAMPLE13_ELEMENTS,
    NUM_SAMPLE12_ELEMENTS,
    NUM_SAMPLE23_ELEMENTS,
    NUM_SAMPLE22_ELEMENTS,
    NUM_SAMPLE10_ELEMENTS,
    NUM_SAMPLE9_ELEMENTS,
    NUM_SAMPLE8_ELEMENTS,
    NUM_SAMPLE15_ELEMENTS,
    NUM_SAMPLE24_ELEMENTS,
    NUM_SAMPLE25_ELEMENTS
};
uint32_t sample_i = 0;

void play_sample(struct audio_buffer_pool *ap) {
    struct audio_buffer *buffer = take_audio_buffer(ap, true);
    int16_t *samples = (int16_t *)buffer->buffer->bytes;
    for (uint i = 0; i < buffer->max_sample_count; i++) {
        samples[i] = recordings[recording_i][sample_i++];
        if (sample_i >= recording_num_samples[recording_i]) {
            sample_i = 0;
        }
    }
    buffer->sample_count = buffer->max_sample_count;
    give_audio_buffer(ap, buffer);
}

void next_sample() {
    recording_i++;
    if (recording_i >= NUM_RECORDINGS) {
        recording_i = 0;
    }
    sample_i = 0;
}

void set_sample_num(uint8_t i) { 
    recording_i = i; 
    sample_i = 0;
}
uint8_t get_sample_num() { 
    return recording_i; 
}
