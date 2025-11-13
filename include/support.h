#ifndef SUPPORT_H
#define SUPPORT_H

#include <math.h>
#include <stdint.h>

#define N 1000 // Size of the wavetable
short int wavetable[N];

#define RATE 20000

// defined as extern here so that we can share it between
// support.c and pwm.c, where they are included.
extern int step0;
extern int offset0;

void init_wavetable();
void set_freq(float f);

#endif