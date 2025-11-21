#ifndef SUPPORT_H
#define SUPPORT_H

#include <math.h>
#include <stdint.h>

#define N 32768 // Size of the wavetable
#define M 13000 // Size of the Amplitude Envelope

short int wavetable[N];
double envelope[M];

#define RATE 20000 //Audio Sample Rate

// defined as extern here so that we can share it between
// support.c and pwm.c, where they are included.
extern uint32_t step0;
extern uint32_t offset0;
extern uint32_t step1;
extern uint32_t offset1;
extern uint32_t step2;
extern uint32_t offset2;
extern uint32_t step3;
extern uint32_t offset3;

//void init_wavetable();
void init_wavetable();
void init_envelope();
void set_freq(int chan, float f);

#endif