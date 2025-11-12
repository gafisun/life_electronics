#include <stdint.h>
#define DISPLAY_WIDTH  32
#define DISPLAY_HEIGHT 32

void display_init(void);

void display_start(void);

void display_stop(void);

void display_refresh(void);

void display_set_brightness(uint8_t percent);

void display_clear(void);

void display_set_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);


/*
int main() {
    stdio_init_all();
    display_init();
    display_set_brightness(50);

    while (1) {
        display_clear();
        
        // Draw some pixels
        display_set_pixel(0, 0, 1, 0, 0);
        display_set_pixel(31, 31, 0, 1, 0)
        
        display_refresh();
        
        sleep_ms(16);
    }
}
*/