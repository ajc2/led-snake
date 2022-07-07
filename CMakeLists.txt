cmake_minimum_required(VERSION 3.13)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(pico-sdk/pico_sdk_init.cmake)

# initialize the Raspberry Pi Pico SDK
project(led_snake)
pico_sdk_init()
add_executable(led_snake main.c)

pico_enable_stdio_uart(led_snake 1)
pico_add_extra_outputs(led_snake)

target_link_libraries(led_snake pico_stdlib hardware_spi pico_time pico_malloc)
