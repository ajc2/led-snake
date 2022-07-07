#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/malloc.h"
#include "hardware/spi.h"

#ifndef LIB_LED
#define LIB_LED

typedef struct led {
	spi_inst_t *spi;
	uint8_t orientation;
	uint8_t cs_pin;
	uint8_t modules;
	bool write_order;
	unsigned char *font;

	uint8_t *_fb;
} led;

// write a command to the LED controller over SPI
// this will send a command to the FIRST in the chain only
int led_write(led *led, uint8_t address, uint8_t data) {
	uint8_t buf[2];
	buf[0] = address;
	buf[1] = data;

	spi_write_blocking(led->spi, buf, 2);

	return 0;
}

// write a command to ALL LED controllers
// used to set the same command on all
int led_write_all(led *led, uint8_t address, uint8_t data) {
	int i;
	for(i = 0; i < led->modules; i++) {
		led_write(led, address, data);
	}

	return 0;
}

// pull chip select low and sleep briefly
int led_begin(led *led) {
	gpio_put(led->cs_pin, 0);
	sleep_us(10);
}

// pull chip select high and sleep briefly
int led_end(led *led) {
	gpio_put(led->cs_pin, 1);
	sleep_us(10);
}

// enable the LEDs (exit shutdown mode)
int led_enable(led *led) {
	led_begin(led);
	led_write_all(led, 0xC, true);
	led_end(led);
	//sleep_us(250);
}

// shutdown the LEDs
int led_shutdown(led *led) {
	led_begin(led);
	led_write_all(led, 0xC, false);
	led_end(led);
	//sleep_us(250);
}

// enable or disable test mode (all lights on)
int led_test_mode(led *led, bool enable) {
	led_begin(led);
	led_write_all(led, 0xF, enable);
	led_end(led);
}

// set the scan limit register
int led_set_scan_limit(led *led, uint8_t scan) {
	led_begin(led);
	led_write_all(led, 0xB, scan);
	led_end(led);
}

// set intensity (brightness) of the LED controllers
int led_set_intensity(led *led, uint8_t intensity) {
	led_begin(led);
	led_write_all(led, 0xA, intensity);
	led_end(led);
}

// fill the framebuffer with off or on pixels
int led_clear(led *led, bool on) {
	int val;
	if(on) {
		val = 0b11111111;
	}
	else {
		val = 0b00000000;
	}

	memset(led->_fb, val, (size_t)8*led->modules);
}

// change the value of one pixel
int led_put(led *led, size_t x, size_t y, uint8_t state) {
	if(x < 0 || x > led->modules*8-1 || y < 0 || y > 7) {
		return 1;
	}

	int idx, bit, mask;
	idx = (x&0xFFFFFFF8)+(y&7);

	switch(state) {
		case 0:
			mask = ~(128>>(x&7));
			led->_fb[idx] &= mask;
			break;
		case 1:
			bit = 128>>(x&7);
			led->_fb[idx] |= bit;
			break;
		case 2:
			bit = 128>>(x&7);
			led->_fb[idx] ^= bit;
			break;
	}

	return 0;
}

// fill a rectangle
int led_fill(led *led, size_t x, size_t y, size_t width, size_t height, uint8_t state) {
	size_t u, v;

	for(v = y; v < y+height; v++) {
		for(u = x; u < x+width; u++) {
			led_put(led, u, v, state);
		}
	}

	return 0;
}

// draw a single character on the LED display
int led_put_chr(led *led, size_t x, size_t y, unsigned char chr, uint8_t state) {
	size_t b, c, u, v;
	uint8_t f, p;

	if(chr > 255) { return 1; }
	c = ((size_t)chr)*3;
	for(b = 0; b < 3; b++) {
		f = led->font[c+b];
		for(p = 0; p < 8; p++) {
			if(f&(128>>p)) {
				u = x+(p&3);
				v = y+(p>>2&1)+b*2;

				led_put(led, u, v, state);
			}
		}
	}
}

// draw a string on the LED display using a 4x6 font
// the 4x6 font is a packed byte string where each byte
// is two rows of a character, the high nybble is the first row
// and the low nybble is the second. each char is thus 3 bytes
// from top to bottom, left to right, hight to low.
int led_put_str(led *led, size_t x, size_t y, unsigned char *str, size_t len, uint8_t state) {
	size_t i;

	for(i = 0; i < len; i++) {
		led_put_chr(led, x+i*4, y, str[i], state);
	}

	return 0;
}

// send the contents of the framebuffer to the display
int led_render(led *led) {
	int i, j, start, end, ofs, idx;

	for(i = 0; i < 8; i++) {
		led_begin(led);
		for(j = 0; j < led->modules; j++) {
			idx = j*8;
			if(led->write_order) { idx = (led->modules-1)*8-idx; }
			idx += i;
			led_write(led, i+1, led->_fb[idx]);
		}
		led_end(led);
	}
}

// initialize an LED controller
led led_init(spi_inst_t *spi, uint8_t cs, uint8_t mods, bool order, unsigned char *font) {
	led led = {
		.spi = spi,
		.cs_pin = cs,
		.modules = mods,
		.orientation = 0,
		.write_order = order,
		.font = font,

		._fb = (uint8_t *)malloc((size_t)8*mods)
	};

	// wakeup
	led_enable(&led);
	// scan all lines
	led_set_scan_limit(&led, 0b111);
	// set lowest intensity
	led_set_intensity(&led, 0);
	// clear digit registers
	led_clear(&led, false);
	led_render(&led);

	return led;
}

#endif
