#ifndef SUPPORT_H
#define SUPPORT_H

#include <math.h>
#include <stdint.h>

// we defined this in main.c
#define N 1000 // Size of the wavetable
short int wavetable[N];

#define RATE 20000

// defined as extern here so that we can share it between
// support.c and main.c, where they are included.
extern int step0;
extern int offset0;
extern int step1;
extern int offset1;

// Part 3: Analog-to-digital conversion for a volume level.
extern int volume;

void init_wavetable();
void set_freq(int chan, float f);

#endif