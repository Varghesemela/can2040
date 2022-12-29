
add_executable(${PROJECT_NAME} main.c can2040.c can2040.h)


# target_link_libraries(${PROJECT_NAME} PUBLIC
#     cmsis_core
#     pico_stdlib
#     pico_multicore
#     pico_unique_id
#     hardware_sync
#     hardware_pio
#     hardware_dma
#     hardware_irq
#     hardware_i2c
#     hardware_uart
#     hardware_flash
#     hardware_pwm
#     hardware_watchdog
#     hardware_adc
#     )


pico_add_extra_outputs(${PROJECT_NAME})
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)