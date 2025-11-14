#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define MATRIX_WIDTH   32
#define MATRIX_HEIGHT  32

// Initialize GPIO and internal framebuffer
void display_init(void);

// Fill the whole framebuffer with a solid color
void display_fill(uint16_t color);

// Set one pixel in the framebuffer (no bounds = no-op)
void display_set_pixel(int x, int y, uint16_t color);

// Draw a simple test pattern (gradients + a few shapes)
void display_draw_test_pattern(void);

// Perform one full refresh scan of the matrix
// Call this repeatedly in your main loop.
void display_refresh_once(void);

// Helper: pack 8-bit RGB into 16-bit 5-6-5
uint16_t display_color565(uint8_t r, uint8_t g, uint8_t b);

#endif
