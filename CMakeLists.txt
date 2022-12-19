cmake_minimum_required(VERSION 3.13)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_SDK_PATH "/home/$ENV{USER}/pico/pico-sdk")
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(can2040 C CXX ASM)

pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        -Wno-dev
        )

add_executable(${PROJECT_NAME})
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/can2040.pio)
include_directories(src)

target_sources(${PROJECT_NAME} PUBLIC src/main.c src/can2040.h src/can2040.c)


target_link_libraries(${PROJECT_NAME} PRIVATE
        cmsis_core
        pico_stdlib
        pico_multicore
        hardware_pio
        hardware_dma
        hardware_irq
        hardware_uart
        hardware_spi
        hardware_i2c
        hardware_flash
        hardware_sync
        hardware_watchdog
        hardware_pwm
        )

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
# pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})

# add url via pico_set_program_url
# example_auto_set_url(${PROJECT_NAME})
