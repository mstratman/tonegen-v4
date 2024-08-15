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

void tone_init();
void play_tone(struct audio_buffer_pool *ap);

void set_tone_speed(uint16_t potval); // 0 to MAX_POT

// The "tone_num" represents the current pattern or tone that is playing. 
// It changes each time you press the "tone gen" button.
void next_tone();
void set_tone_num(uint8_t i);
uint8_t get_tone_num();