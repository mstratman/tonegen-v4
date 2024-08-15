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


typedef struct {
    uint8_t mode;
    uint8_t sample_num;
    uint8_t tone_num;
} settings_t;

settings_t flash_read_settings();
void flash_update_settings(settings_t settings);

