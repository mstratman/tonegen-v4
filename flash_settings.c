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

// This treats the flash memory kind of like EEPROM. We're going to use the last
// flash sector (4k) to store program data.
//
// WARNING: You cannot write 0xFF (255) with this.
//
// for more info see
// https://www.makermatrix.com/blog/read-and-write-data-with-the-pi-pico-onboard-flash/
//
// NOTE: EVERYTHING about this is hard-coded to the rp2040.

#include <hardware/flash.h>
#include <hardware/sync.h>
#include <string.h>

#include "flash_settings.h"
#include "debug.h"

// end - 4096. This doesn't include the XIP_BASE, which the flash_*() functions
// don't need
#define FLASH_LAST_SECTOR (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

// XIP_BASE is execute in place (see pg24: ROM, XIP, SRAM, ...)
#define XIP_FLASH_START (XIP_BASE + FLASH_LAST_SECTOR)
#define XIP_FLASH_END   (XIP_BASE + PICO_FLASH_SIZE_BYTES)

#define PAGES_PER_SECTOR (FLASH_SECTOR_SIZE / FLASH_PAGE_SIZE)

settings_t flash_read_settings() {
    P("flash_read_settings\n");
    settings_t rv = {0, 0, 0};

    uint8_t *p;

    // Erased flash has 0xFF in each byte, so find the last
    // non-0xFF value
    for (uint32_t addr = XIP_FLASH_START; addr < XIP_FLASH_END; addr++) {
        p = (uint8_t *)addr;
        // either the memory has never been saved to, or we went too far
        // and the previous 3 bytes are our settings.
        if (*p == 0xFF) {
            if (addr >= 3) {
                rv.mode =       *(uint8_t *)(addr - 3);
                rv.sample_num = *(uint8_t *)(addr - 2);
                rv.tone_num =   *(uint8_t *)(addr - 1);
            }
            break;
        }
    }

    return rv;
}

// Taking advantage of this: Individual bytes can have any bit changed from 1 to
// 0 without requiring erasure.
// https://forums.raspberrypi.com/viewtopic.php?t=369026&e=1
void flash_update_settings(settings_t settings) {
    P("flash_update_setting\n");

    // since we will ultimately write an entire page anyway, let's prepare a
    // page at a time too
    uint8_t buffer[FLASH_PAGE_SIZE];
    uint8_t found_end = 0;
    uint8_t page;
    uint8_t *p;
    uint16_t addr_offset;

    for (page = 0; page < PAGES_PER_SECTOR; page++) {
        PF("reading page=%d\n", page);

        // clear the buffer
        memset(buffer, 0xFF, FLASH_PAGE_SIZE);

        for (addr_offset = 0; addr_offset < FLASH_PAGE_SIZE; addr_offset++) {
            uint32_t addr = XIP_FLASH_START + (page * FLASH_PAGE_SIZE) + addr_offset;

            p = (uint8_t *)addr;
            buffer[addr_offset] = *p;

            PF("addr_offset=%d p=%d\n", addr_offset, *p);

            if (*p == 0xFF) {
                if ((addr_offset + 2) < FLASH_PAGE_SIZE) {
                    PF("writing to page=%d addr_offset=%d", page, addr_offset);
                    found_end = 1;
                    buffer[addr_offset] = settings.mode;
                    buffer[addr_offset + 1] = settings.sample_num;
                    buffer[addr_offset + 2] = settings.tone_num;
                    break;
                    // there's a couple bytes at the end of the page, let's zero
                    // them out to prevent flash_read_settings() from thinking
                    // it found the settings
                } else {
                    for (uint16_t i = addr_offset; i < FLASH_PAGE_SIZE; i++) {
                        buffer[i] = 0;
                    }
                    break;
                }
            }
        }
        P("Done with page\n");
        if (found_end) break;
    }

    uint32_t save_interrupts = save_and_disable_interrupts();

    // is the sector full?
    if (!found_end) {
        P("************Erasing last flash sector**************\n");
        // Erase the last sector of the flash
        flash_range_erase(FLASH_LAST_SECTOR, FLASH_SECTOR_SIZE);

        page = 0;
        memset(buffer, 0xFF, FLASH_PAGE_SIZE);
        buffer[0] = settings.mode;
        buffer[1] = settings.sample_num;
        buffer[2] = settings.tone_num;
    }

    PF("----- Writing flash page=%d addr=%d\n", page, (FLASH_LAST_SECTOR + page * FLASH_PAGE_SIZE));
    PF("buffer[0]=%d buffer[4]=%d\n", buffer[0], buffer[4]);
    flash_range_program(FLASH_LAST_SECTOR + page * FLASH_PAGE_SIZE,
                        (uint8_t *)buffer, FLASH_PAGE_SIZE);

    uint32_t newaddr = XIP_BASE + (FLASH_LAST_SECTOR + page * FLASH_PAGE_SIZE);
    PF("XIP_BASE=%d + addr=%d == newaddr=%d\n", XIP_BASE, (FLASH_LAST_SECTOR + page * FLASH_PAGE_SIZE), newaddr);
    PF("read back buffer[0]=%d buffer[4]=%d\n", *(uint8_t *)(newaddr), *(uint8_t *)(newaddr+4));
    restore_interrupts(save_interrupts);
}
