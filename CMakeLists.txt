cmake_minimum_required(VERSION 3.5)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/Users/mstratman/RP2040/pico-sdk")



# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR} )
set(PICO_BOARD_CMAKE_DIRS ${CMAKE_CURRENT_LIST_DIR} )

set(PICO_BOARD towerboard CACHE STRING "Board type")
set(PICO_PLATFORM rp2040)

# pico-extras has i2s audio code
set(PICO_EXTRAS_PATH, "/Users/mstratman/Projects/pedals/diy-pedal-tower/tone-generator/tonegen-v4/pico-extras")
include(pico_extras_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(tonegen-v4 C CXX ASM)


# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(tonegen-v4 main.c button.c sample_player.c tone_player.c flash_settings.c)


pico_set_program_name(tonegen-v4 "tonegen-v4")
pico_set_program_version(tonegen-v4 "4.2")

pico_enable_stdio_uart(tonegen-v4 1)
pico_enable_stdio_usb(tonegen-v4 0)

# Add the standard library to the build
target_link_libraries(tonegen-v4 pico_stdlib pico_audio_i2s hardware_adc hardware_flash)

# Add the standard include files to the build
target_include_directories(tonegen-v4 PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)


pico_set_linker_script(tonegen-v4 ${CMAKE_SOURCE_DIR}/memmap_custom.ld)


pico_add_extra_outputs(tonegen-v4)

target_compile_definitions(tonegen-v4 PRIVATE
  PICO_AUDIO_I2S_MONO_OUTPUT=1
  PICO_AUDIO_I2S_MONO_INPUT=1
  #Production pins:
  PICO_AUDIO_I2S_DATA_PIN=24
  PICO_AUDIO_I2S_CLOCK_PIN_BASE=22
  ## pi pico pins:
  #PICO_AUDIO_I2S_DATA_PIN=12
  #PICO_AUDIO_I2S_CLOCK_PIN_BASE=10
)

