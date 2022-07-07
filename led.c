#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/malloc.h"
#include "hardware/spi.h"

#ifndef LIB_LED
#define LIB_LED

const unsigned char *LED_DEFAULT_FONT = "\x00\x00\x00\x00\x01\x10\x00\x02\x20\x00\x03\x30\x00\x04\x40\x00\x05\x50\x00\x06\x60\x00\x07\x70\x00\x08\x80\x00\x09\x90\x00\x0A\xA0\x00\x0B\xB0\x00\x0C\xC0\x00\x0D\xD0\x00\x0E\xE0\x00\x0F\xF0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x44\x40\x40\xAA\x00\x00\xAE\xAE\xA0\x4E\xC6\xE4\xCA\x4A\x60\x68\x4E\xE0\x44\x00\x00\x48\x88\x84\x42\x22\x24\x4E\xA0\x00\x04\xE4\x00\x00\x02\x24\x00\xE0\x00\x00\x0C\xC0\x22\x44\x88\xEA\xAA\xE0\x4C\x44\xE0\xE2\xE8\xE0\xC2\x42\xC0\xAA\xE2\x20\xE8\xC2\xC0\x68\xCA\xC0\xE2\x24\x40\xEA\x4A\xE0\xEA\xE2\xC0\xCC\x0C\xC0\xCC\x0C\x48\x24\x84\x20\x0E\x0E\x00\x84\x24\x80\x4A\x20\x40\x4A\xEE\x86\x4A\xEA\xA0\xCA\xCA\xC0\x68\x88\x60\xCA\xAA\xC0\xE8\xC8\xE0\xE8\xC8\x80\x68\xEA\x60\xAA\xEA\xA0\xE4\x44\xE0\xE2\x22\xC0\xAA\xCA\xA0\x88\x88\xE0\xAE\xAA\xA0\xEA\xAA\xA0\x4A\xAA\x40\xCA\xC8\x80\x4A\xAE\x60\xCA\xCA\xA0\x68\x42\xC0\xE4\x44\x40\xAA\xAA\xE0\xAA\xAE\x40\xAA\xAE\xA0\xAA\x4A\xA0\xAA\x44\x40\xE2\x48\xE0\xE8\x88\x8E\x88\x44\x22\xE2\x22\x2E\x4A\x00\x00\x00\x00\x0F\x84\x00\x00\x04\xAA\x60\x88\xCA\xC0\x06\x88\x60\x22\x6A\x60\x4A\xE8\x60\x68\xC8\x80\x06\xA6\x2C\x08\xCA\xA0\x40\xC4\x40\x20\x22\xC0\x08\xAC\xA0\x08\x88\x60\x0A\xEE\xA0\x0C\xAA\xA0\x04\xAA\x40\x0C\xAC\x88\x06\xA6\x22\x06\x88\x80\x06\x86\xC0\x04\xE4\x60\x0A\xAA\x60\x0A\xAA\x40\x0A\xAE\xA0\x00\xA4\xA0\x0A\x62\xC0\x0E\x2C\xE0\x64\x88\x46\x44\x44\x44\xC4\x22\x4C\x5A\x00\x00\x77\x0F\xF0";

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
int led_put_char(led *led, size_t x, size_t y, unsigned char chr, uint8_t state) {
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
		led_put_char(led, x+i*4, y, str[i], state);
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
