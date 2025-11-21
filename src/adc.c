#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

#include <assert.h>
#define ADC_PIN 45

void adc_init_pot(void){
    static bool inited = false;
    if (!inited) {
        adc_init();
        inited = true;
    }
    adc_gpio_init(ADC_PIN);
    assert(ADC_PIN >= 40 && ADC_PIN <= 47);
    adc_select_input(ADC_PIN - 40);
}

uint16_t adc_read_u12(void){ // one 12-bit reading
    adc_select_input(ADC_PIN - 40);
    (void)adc_read();
    return adc_read();
}

uint16_t adc_read_avg_u12(int samples){ // samples - number of ADC readings we want to collect
    uint32_t s = 0;
    for (int i = 0; i < samples; ++i) {
        s += adc_read_u12();
    }
    return (uint16_t)(s / (uint32_t)samples);
}

uint16_t adc_scale_u12(uint16_t v, uint16_t out_min, uint16_t out_max){
    return (uint16_t)(out_min + ((uint32_t)v * (out_max - out_min)) / 4095u);
}


// in the main we should use
// #define ADC_SAMPLES 8
// #define GOL_DELAY_MIN_MS 30
// #define GOL_DELAY_MAX_MS 800

// adc_init_pot();
// uint16_t v = adc_read_avg_u12(ADC_SAMPLES);
// uint16_t delay_ms = adc_scale_u12(v, GOL_DELAY_MIN_MS, GOL_DELAY_MAX_MS);


/*
I’m using GPIO26 (ADC0) to read the analog voltage from the pot, and I wrote adc.c and adc.h files modeled after the structure we used in previous labs.
The code initializes the ADC once, performs averaged 12-bit readings, and scales the raw value to a delay range (about 30–800 ms) to control the Game of Life simulation speed.
Next, I’ll integrate it into the main loop so the simulation dynamically adjusts speed without blocking other peripherals.
*/