#include "display.h"

#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#define PIN_R1   8   // Upper half red
#define PIN_G1   9   // Upper half green
#define PIN_B1   10   // Upper half blue
#define PIN_R2   11   // Lower half red
#define PIN_G2   12   // Lower half green
#define PIN_B2   13   // Lower half blue

#define PIN_A    15   // Row address A
#define PIN_B    16  // Row address B
#define PIN_C    17  // Row address C
#define PIN_D    18  // Row address D

#define PIN_CLK  14  // Clock
#define PIN_LAT  19  // Latch (STB)
#define PIN_OE   20  // Output enable (active low)

#define ROW_ON_TIME_US  250 

static uint16_t framebuffer[MATRIX_WIDTH * MATRIX_HEIGHT];

uint16_t display_color565(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t r5 = (uint16_t)(r & 0xF8) << 8; // upper 5 bits
    uint16_t g6 = (uint16_t)(g & 0xFC) << 3; // upper 6 bits
    uint16_t b5 = (uint16_t)(b >> 3);        // upper 5 bits
    return (uint16_t)(r5 | g6 | b5);
}

static inline void set_addr_lines(uint8_t row) {
    gpio_put(PIN_A, (row >> 0) & 0x1);
    gpio_put(PIN_B, (row >> 1) & 0x1);
    gpio_put(PIN_C, (row >> 2) & 0x1);
    gpio_put(PIN_D, (row >> 3) & 0x1);
}

static inline void pulse_clk(void) {
    gpio_put(PIN_CLK, 1);
    sleep_us(1);
    gpio_put(PIN_CLK, 0);
}


void display_init(void) {
    const uint PIN_LIST[] = {
        PIN_R1, PIN_G1, PIN_B1,
        PIN_R2, PIN_G2, PIN_B2,
        PIN_A, PIN_B, PIN_C, PIN_D,
        PIN_CLK, PIN_LAT, PIN_OE
    };

    for (size_t i = 0; i < sizeof(PIN_LIST)/sizeof(PIN_LIST[0]); ++i) {
        gpio_init(PIN_LIST[i]);
        gpio_set_dir(PIN_LIST[i], GPIO_OUT);
    }

    gpio_put(PIN_CLK, 0);
    gpio_put(PIN_LAT, 0);
    gpio_put(PIN_OE, 1); // OE=1 => LEDs OFF

    display_fill(0); // clear framebuffer
}

void display_fill(uint16_t color) {
    for (int i = 0; i < MATRIX_WIDTH * MATRIX_HEIGHT; ++i) {
        framebuffer[i] = color;
    }
}

void display_set_pixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= MATRIX_WIDTH ||
        y < 0 || y >= MATRIX_HEIGHT) {
        return;
    }
    framebuffer[y * MATRIX_WIDTH + x] = color;
}
void display_draw_test_pattern(void) {
    display_fill(display_color565(0, 0, 0));

    uint16_t band_colors[8] = {
        display_color565(255,   0,   0), 
        display_color565(255, 255,   0),
        display_color565(0,   255,   0), 
        display_color565(0,   255, 255),
        display_color565(0,     0, 255), 
        display_color565(255,   0, 255), 
        display_color565(255, 255, 255),
        display_color565(0,     0,   0)
    };

    for (int x = 0; x < MATRIX_WIDTH; ++x) {
        int band = (x * 8) / MATRIX_WIDTH;
        uint16_t c = band_colors[band];
        for (int y = 0; y < MATRIX_HEIGHT; ++y) {
            display_set_pixel(x, y, c);
        }
    }
}


void display_refresh_once(void) {
    for (int row = 0; row < MATRIX_HEIGHT / 2; ++row) {
        gpio_put(PIN_OE, 1); // LEDs off

        set_addr_lines((uint8_t)row);

        for (int x = 0; x < MATRIX_WIDTH; ++x) {
            int upper_index = row * MATRIX_WIDTH + x;
            int lower_index = (row + MATRIX_HEIGHT / 2) * MATRIX_WIDTH + x;

            uint16_t c1 = framebuffer[upper_index];
            uint16_t c2 = framebuffer[lower_index];

            bool r1 = (c1 & 0xF800) != 0;
            bool g1 = (c1 & 0x07E0) != 0;
            bool b1 = (c1 & 0x001F) != 0;

            bool r2 = (c2 & 0xF800) != 0;
            bool g2 = (c2 & 0x07E0) != 0;
            bool b2 = (c2 & 0x001F) != 0;

            gpio_put(PIN_R1, r1);
            gpio_put(PIN_G1, g1);
            gpio_put(PIN_B1, b1);

            gpio_put(PIN_R2, r2);
            gpio_put(PIN_G2, g2);
            gpio_put(PIN_B2, b2);

            pulse_clk();
        }

        gpio_put(PIN_LAT, 1);
        sleep_us(1);
        gpio_put(PIN_LAT, 0);

        gpio_put(PIN_OE, 0); // LEDs on
        sleep_us(ROW_ON_TIME_US);
        gpio_put(PIN_OE, 1); // LEDs off before changing row
    }
}
