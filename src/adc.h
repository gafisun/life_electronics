#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void adc_init_pot(void);
uint16_t adc_read_u12(void);
uint16_t adc_read_avg_u12(int samples);
uint16_t adc_scale_u12(uint16_t v, uint16_t out_min, uint16_t out_max);

#endif