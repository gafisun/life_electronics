#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/xosc.h"
#include "pico/multicore.h"
#include "display.h"

const int DISPLAY_R1 = -1; // Display R1 gpio pin
const int DISPLAY_B1 = -1; // Display R1 gpio pin
const int DISPLAY_R2 = -1; // Display R1 gpio pin
const int DISPLAY_B2 = -1; // Display R1 gpio pin
const int DISPLAY_A = -1; // Display R1 gpio pin
const int DISPLAY_C = -1; // Display R1 gpio pin
const int DISPLAY_CLK = -1; // Display R1 gpio pin
const int DISPLAY_OE = -1; // Display R1 gpio pin
const int DISPLAY_G1 = -1; // Display R1 gpio pin
const int DISPLAY_G2 = -1; // Display R1 gpio pin
const int DISPLAY_B = -1; // Display R1 gpio pin
const int DISPLAY_D = -1; // Display R1 gpio pin
const int DISPLAY_LAT = -1; // Display R1 gpio pin

// 1-bit per color per pixel. framebuffer[y][x] bits: 0=R, 1=G, 2=B
static uint8_t framebuffer[DISPLAY_HEIGHT][DISPLAY_WIDTH];

// Display state
static uint16_t row_hold_us = 80;
static bool scanning_enabled = false;

static inline uint32_t addr_bits_for_row(uint8_t row) {
    uint32_t v = 0;
    if (row & 0x1) v |= (1u << DISPLAY_A);
    if (row & 0x2) v |= (1u << DISPLAY_B);
    if (row & 0x4) v |= (1u << DISPLAY_C);
    if (row & 0x8) v |= (1u << DISPLAY_D);
    return v;
}

static inline uint32_t data_bits_for_pixels(uint8_t top, uint8_t bottom) {
    uint32_t v = 0;
    if (top    & 0x01) v |= (1u << DISPLAY_R1);
    if (top    & 0x02) v |= (1u << DISPLAY_G1);
    if (top    & 0x04) v |= (1u << DISPLAY_B1);
    if (bottom & 0x01) v |= (1u << DISPLAY_R2);
    if (bottom & 0x02) v |= (1u << DISPLAY_G2);
    if (bottom & 0x04) v |= (1u << DISPLAY_B2);
    return v;
}

void display_init(){
    uint gpio_mask = (1u << DISPLAY_R1) | 
                     (1u << DISPLAY_B1) | 
                     (1u << DISPLAY_R2) | 
                     (1u << DISPLAY_B2) | 
                     (1u << DISPLAY_A) | 
                     (1u << DISPLAY_C) | 
                     (1u << DISPLAY_CLK) | 
                     (1u << DISPLAY_OE) | 
                     (1u << DISPLAY_G1) | 
                     (1u << DISPLAY_G2) | 
                     (1u << DISPLAY_B) | 
                     (1u << DISPLAY_D) | 
                     (1u << DISPLAY_LAT);
    gpio_init_mask(gpio_mask);
    gpio_set_dir_masked(gpio_mask, GPIO_OUT);

    gpio_put(DISPLAY_OE, 1);
    gpio_put(DISPLAY_CLK, 0);
    gpio_put(DISPLAY_LAT, 0);

    display_clear();
}

void display_start(){
    scanning_enabled = true;
}

void display_stop(){
    scanning_enabled = false;
    gpio_put(DISPLAY_OE, 1);
}

void display_refresh(void) {
    if (!scanning_enabled) return;

    uint32_t mask_addr = (1u << DISPLAY_A) | (1u << DISPLAY_B) | 
                         (1u << DISPLAY_C) | (1u << DISPLAY_D);
    uint32_t mask_data = (1u << DISPLAY_R1) | (1u << DISPLAY_G1) | (1u << DISPLAY_B1) |
                         (1u << DISPLAY_R2) | (1u << DISPLAY_G2) | (1u << DISPLAY_B2);

    for (uint8_t row = 0; row < DISPLAY_HEIGHT / 2; ++row) {
        gpio_put(DISPLAY_OE, 1);

        gpio_put_masked(mask_addr, addr_bits_for_row(row));

        for (uint8_t x = 0; x < DISPLAY_WIDTH; ++x) {
            uint8_t top    = framebuffer[row][x];
            uint8_t bottom = framebuffer[row + DISPLAY_HEIGHT/2][x];

            gpio_put_masked(mask_data, data_bits_for_pixels(top, bottom));

            gpio_put(DISPLAY_CLK, 1);
            gpio_put(DISPLAY_CLK, 0);
        }

        gpio_put(DISPLAY_LAT, 1);
        gpio_put(DISPLAY_LAT, 0);

        gpio_put(DISPLAY_OE, 0);
        if (row_hold_us) {
            sleep_us(row_hold_us);
        }
    }
}

void display_set_brightness(uint8_t percent) {
    if (percent > 100) percent = 100;
    row_hold_us = (uint16_t)((percent * 200u) / 100u);
}

void display_clear(void) {
    memset(framebuffer, 0, sizeof(framebuffer));
}

void display_set_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
    uint8_t v = (r ? 1u : 0u) | (g ? 2u : 0u) | (b ? 4u : 0u);
    framebuffer[y][x] = v;
}