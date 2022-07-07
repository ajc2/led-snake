#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/malloc.h"
#include "hardware/spi.h"

#include "./led.c"

#define PIN_TX PICO_DEFAULT_SPI_TX_PIN
#define PIN_CLK PICO_DEFAULT_SPI_SCK_PIN
#define PIN_CS PICO_DEFAULT_SPI_CSN_PIN

#define PIN_BTN_L 21
#define PIN_BTN_R 20

bool test_left_button() {
	return gpio_get(PIN_BTN_L) == 0;
}

bool test_right_button() {
	return gpio_get(PIN_BTN_R) == 0;
}

bool test_any_button() {
	return test_left_button() || test_right_button();
}

void wait_any_button() {
	do {} while(!test_any_button());
}

int main() {
	// enable stdio
	stdio_init_all();

	// configure GPIO pins
	gpio_set_function(PIN_CLK, GPIO_FUNC_SPI);
	gpio_set_function(PIN_TX, GPIO_FUNC_SPI);
	// LED chip select pin is active low
	gpio_init(PIN_CS);
	gpio_set_dir(PIN_CS, GPIO_OUT);
	gpio_pull_up(PIN_CS);
	gpio_put(PIN_CS, 1);
	// set button pins
	gpio_init(PIN_BTN_L);
	gpio_set_dir(PIN_BTN_L, GPIO_IN);
	gpio_pull_up(PIN_BTN_L);
	gpio_init(PIN_BTN_R);
	gpio_set_dir(PIN_BTN_R, GPIO_IN);
	gpio_pull_up(PIN_BTN_R);

	// 1MHz
	spi_init(spi_default, 1000000);
	sleep_ms(100);

	led disp = led_init(spi_default, PIN_CS, 4, false, LED_DEFAULT_FONT);

	while(true) {
		led_clear(&disp, false);
		if(test_left_button()) {
			led_put_char(&disp, 8, 1, 'L', 1);
		}
		if(test_right_button()) {
			led_put_char(&disp, 20, 1, 'R', 1);
		}
		led_render(&disp);
		sleep_us(16666);
	}

	return 0;
}
