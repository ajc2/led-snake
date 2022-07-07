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

	// 1MHz
	spi_init(spi_default, 1000000);
	sleep_ms(100);

	led disp = led_init(spi_default, PIN_CS, 4, false, (unsigned char *)NULL);

	return 0;
}
