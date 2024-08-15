/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// -----------------------------------------------------
// NOTE: THIS HEADER IS ALSO INCLUDED BY ASSEMBLER SO
//       SHOULD ONLY CONSIST OF PREPROCESSOR DIRECTIVES
// -----------------------------------------------------

// This header may be included by other board headers as "boards/pico.h"

#ifndef _BOARDS_TOWER_H
#define _BOARDS_TOWER_H

// For board detection
#define RASPBERRYPI_PICO

// --- UART ---
#ifndef PICO_DEFAULT_UART
#define PICO_DEFAULT_UART 0
#endif
#ifndef PICO_DEFAULT_UART_TX_PIN
#define PICO_DEFAULT_UART_TX_PIN 0
#endif
#ifndef PICO_DEFAULT_UART_RX_PIN
#define PICO_DEFAULT_UART_RX_PIN 1
#endif


#define PICO_XOSC_STARTUP_DELAY_MULTIPLIER 64

// --- FLASH ---

//#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1
#define PICO_BOOT_STAGE2_CHOOSE_GENERIC_03H 1


#ifndef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 2
#endif

#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)
#endif

// Drive high to force power supply into PWM mode (lower ripple on 3V3 at light loads)
//#define PICO_SMPS_MODE_PIN 23
//#ifndef PICO_RP2040_B0_SUPPORTED
//#define PICO_RP2040_B0_SUPPORTED 1
//#endif

// The GPIO Pin used to read VBUS to determine if the device is battery powered.
//#ifndef PICO_VBUS_PIN
//#define PICO_VBUS_PIN 24
//#endif

// The GPIO Pin used to monitor VSYS. Typically you would use this with ADC.
// There is an example in adc/read_vsys in pico-examples.
//#ifndef PICO_VSYS_PIN
//#define PICO_VSYS_PIN 29
//#endif

#endif
