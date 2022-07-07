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
#define LED_MODS 4

// button handling
#define PIN_BTN_L 21
#define PIN_BTN_R 20

bool bounce_l = false;
bool bounce_r = false;

bool test_left_button() {
	if(gpio_get(PIN_BTN_L)==0) {
		if(bounce_l) {
			return false;
		}
		else {
			bounce_l = true;
			return true;
		}
	}
	else {
		bounce_l = false;
		return false;
	}
}

bool test_right_button() {
	if(gpio_get(PIN_BTN_R)==0) {
		if(bounce_r) {
			return false;
		}
		else {
			bounce_r = true;
			return true;
		}
	}
	else {
		bounce_r = false;
		return false;
	}
}

bool test_any_button() {
	return test_left_button() || test_right_button();
}

void wait_any_button() {
	do {} while(!test_any_button());
}


// snake logic
// body buffer
#define WIDTH (LED_MODS*8)
#define HEIGHT 8
#define AREA (WIDTH*HEIGHT)

uint16_t body[AREA];
size_t bodylen;
size_t bodypos;
uint8_t snake_x;
uint8_t snake_y;
uint8_t food_x;
uint8_t food_y;

uint16_t setword(uint8_t x, uint8_t y) {
	return x<<8 | y;
}

void getword(uint16_t word, uint8_t *x, uint8_t *y) {
	*x = word>>8 & 0xF;
	*y = word    & 0xF;
}

void init_game() {
	bodypos = 0;
	bodylen = 1;
	snake_x = WIDTH/2;
	snake_y = HEIGHT/2;
	body[bodypos] = setword(snake_x, snake_y);

	food_x = 0;
	food_y = 0;
}

void run_game() {

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

	led disp = led_init(spi_default, PIN_CS, LED_MODS, false, LED_DEFAULT_FONT);

	while(true) {
		init_game();
		led_put_str(&disp, 6, 1, "PRESS", 5, 1);
		led_render(&disp);
		wait_any_button();
		run_game();
	}

	return 0;
}
