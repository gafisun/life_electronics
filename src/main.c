#include "pico/stdlib.h"
#include "display.h"

int main(void) {
    stdio_init_all();
    sleep_ms(100);

    display_init();
    display_draw_test_pattern();
    // Use this for testing solid color fill
    // display_fill(display_color565(255, 0, 0));
    // display_fill(display_color565(0, 255, 0));
    // display_fill(display_color565(0, 0, 255));

    while (true) {
        display_refresh_once();
    }

    return 0;
}
