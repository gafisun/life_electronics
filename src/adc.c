#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "adc.h"

// Initialize ADC once and switch GPIO26 to analog (pot wiper on GPIO26)
void adc_init_pot(void){
    static bool inited = false;
    if(!inited){ adc_init(); inited = true; }
    adc_gpio_init(26);
}

// One 12-bit reading from ADC0 (0..4095), with a dummy read for settling
uint16_t adc_read_u12(void){
    adc_select_input(0);
    (void)adc_read();
    return adc_read();
}

// Average 'samples' readings to reduce jitter
uint16_t adc_read_avg_u12(int samples){
    uint32_t s = 0;
    for(int i = 0; i < samples; ++i) s += adc_read_u12();
    return (uint16_t)(s / (uint32_t)samples);
}

// Map 0..4095 to [out_min .. out_max] (e.g., 30..800 ms)
uint16_t adc_scale_u12(uint16_t v, uint16_t out_min, uint16_t out_max){
    return (uint16_t)(out_min + ((uint32_t)v * (out_max - out_min)) / 4095u);
}


// in the main we should use
// adc_init_pot();
// uint16_t v = adc_read_avg_u12(8);          // samples = 8
// uint16_t delay_ms = adc_scale_u12(v, 30, 800);  // map to 30..800 ms
/*
#include "pico/stdlib.h"
#include "adc.h"

#define ADC_SAMPLES 8
#define GOL_DELAY_MIN_MS  30
#define GOL_DELAY_MAX_MS 800

int main() {
    stdio_init_all();
    adc_init_pot();

    while (1) {
        uint16_t v = adc_read_avg_u12(ADC_SAMPLES);
        uint16_t delay_ms = adc_scale_u12(v, GOL_DELAY_MIN_MS, GOL_DELAY_MAX_MS);

        // run one Game of Life step here
        // run_game_of_life_step();

        sleep_ms(delay_ms);
    }
}
*/