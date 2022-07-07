#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/malloc.h"
#include "hardware/spi.h"
#include "hardware/adc.h"

#include "./led.c"

#define PIN_TX PICO_DEFAULT_SPI_TX_PIN
#define PIN_CLK PICO_DEFAULT_SPI_SCK_PIN
#define PIN_CS PICO_DEFAULT_SPI_CSN_PIN
#define PIN_ADC 26
#define LED_MODS 4

led disp;

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
#define FRAMETIME 16667
#define SPEED 6
#define SLEEPTIME (SPEED*FRAMETIME)

uint16_t body[AREA];
size_t bodylen;
size_t bodypos;
uint8_t snake_x;
uint8_t snake_y;
uint8_t snake_dir;
uint8_t food_x;
uint8_t food_y;
uint8_t bodyhit = false;

uint16_t setword(uint8_t x, uint8_t y) {
	return ((uint16_t)x<<8) | y;
}

void getword(uint16_t word, uint8_t *x, uint8_t *y) {
	*x = (word>>8) & 0xFF;
	*y = (word)    & 0xFF;
}

void put_food() {
	food_x = rand() % WIDTH;
	food_y = rand() % HEIGHT;
}

// call a function for each body segment
// the callback must return true to continue or false to exit early
// the function also has a generic output parameter
// for returning more data
void each_body(bool (*fn)(uint8_t, uint8_t)) {
	int i, idx;
	uint8_t x, y;
	for(i = 0; i < bodylen; i++) {
		idx = bodypos - i;
		if(idx < 0) {
			idx += AREA;
		}
		getword(body[idx], &x, &y);
		fn(x, y);
	}
}

bool draw_body(uint8_t x, uint8_t y) {
	led_put(&disp, x, y, 1);
	return true;
}

bool test_collision(uint8_t x, uint8_t y) {
	if(snake_x==x && snake_y==y) {
		bodyhit = true;
	}
	else {
		bodyhit = false;
	}
	return !bodyhit;
}

void init_game() {
	bodypos = 0;
	bodylen = 1;
	snake_x = WIDTH/2-1;
	snake_y = HEIGHT/2;
	snake_dir = 0;
	bodyhit = false;
	body[bodypos] = setword(snake_x, snake_y);

	put_food();
}

void run_game() {
	int frames = 0;

	while(true) {
		frames++;

		// # READ BUTTONS #
		if(test_right_button()) {
			snake_dir++;
			if(snake_dir == 4) {
				snake_dir = 0;
			}
		}
		if(test_left_button()) {
			snake_dir--;
			if(snake_dir == 255) {
				snake_dir = 3;
			}
		}

		// # FRAME SLEEP #
		sleep_us(FRAMETIME);
		if(frames < SPEED) {
			continue;
		}

		// # GAME LOGIC #
		// move snake
		switch(snake_dir) {
			case 0:
				snake_x++;
				break;
			case 1:
				snake_y++;
				break;
			case 2:
				snake_x--;
				break;
			case 3:
				snake_y--;
				break;
			default:
				break;
		}
		// out of bounds
		if(snake_x==255 || snake_x>=WIDTH || snake_y==255 || snake_y>=HEIGHT) {
			break;
		}
		// eating
		if(snake_x==food_x && snake_y==food_y) {
			bodylen++;
			put_food();
		}
		// check body collision
		each_body(test_collision);
		if(bodyhit) {break;}
		// update body
		bodypos++;
		if(bodypos >= AREA) {
			bodypos = 0;
		}
		body[bodypos] = setword(snake_x, snake_y);

		// # RENDER #
		led_clear(&disp, false);
		// draw body
		each_body(draw_body);
		// draw food
		led_put(&disp, food_x, food_y, 2);
		led_render(&disp);

		frames = 0;
	}

	// death
	led_fill(&disp, 0, 0, WIDTH, HEIGHT, 2);
	led_render(&disp);
	wait_any_button();
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
	// ADC for PRNG seed
	adc_init();
	adc_gpio_init(PIN_ADC);
	adc_select_input(0);

	// seed PRNG
	unsigned int seed;
	int i;
	for(i = 0; i < 15; i++) {
		uint16_t bits = adc_read() & 0xFF;
		seed = (seed<<4) ^ bits;
	}
	srand(seed);

	// 1MHz
	spi_init(spi_default, 1000000);
	sleep_ms(100);

	disp = led_init(spi_default, PIN_CS, LED_MODS, false, LED_DEFAULT_FONT);

	while(true) {
		init_game();
		led_clear(&disp, false);
		led_put_str(&disp, 6, 1, "PRESS", 5, 1);
		led_render(&disp);
		wait_any_button();
		run_game();
	}

	return 0;
}
