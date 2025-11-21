#include "support.h"


uint32_t step0 = 0;
uint32_t offset0 = 0;
uint32_t step1 = 0;
uint32_t offset1 = 0;
uint32_t step2 = 0;
uint32_t offset2 = 0;
uint32_t step3 = 0;
uint32_t offset3 = 0;


//wave parameters
double A = 1.0;
double beta = 4.0;
double K = 1.4142; //sqrt(2)

void init_wavetable(void) {
    for(int i=0; i < N; i++) {
        double phase = 2.0 * M_PI * i / N;
        double wave_fm = A * sin(phase + (beta * sin(K * phase)));;
        wavetable[i] = (16383 * (wave_fm)) + 16384;
    }
    //potential for different sounds
}

//Amplitude Envelope
const double ATTACK_CUTOFF = .01 * M;
const double ATTACK_MAX = 1;
const double DIP_CUTTOFF = .03 * M;
const double DIP_DOWN = .25;
const double HOLD_CUTTOFF = .3 * M;
const double HOLD_LEVEL = ATTACK_MAX - DIP_DOWN;
const double DECAY_CUTOFF = 1 * M;
const double DECAY_DOWN = HOLD_LEVEL;

void init_envelope(void) {
    for(int i=0; i < M; i++) {
        double wave_en = 0;
        if (i < (int)ATTACK_CUTOFF) {
            wave_en = ATTACK_MAX * (i/ATTACK_CUTOFF);
        } else if (i < (int)DIP_CUTTOFF) {
            wave_en = ATTACK_MAX - (DIP_DOWN * ((i-ATTACK_CUTOFF)/(DIP_CUTTOFF-ATTACK_CUTOFF)));
        } else if (i < (int)HOLD_CUTTOFF) {
            wave_en = HOLD_LEVEL;
        } else  if (i < (int)DECAY_CUTOFF){
            wave_en= HOLD_LEVEL - (DECAY_DOWN * ((i-HOLD_CUTTOFF)/(DECAY_CUTOFF-HOLD_CUTTOFF)));
        }
        envelope[i] = wave_en;
    }
}


void set_freq(int chan, float f) {
    if (chan == 0) {
        if (f == 0.0) {
            step0 = 0;
            offset0 = 0;
        } else
            step0 = (f * N / RATE) * (1<<16);
    }
    if (chan == 1) {
        if (f == 0.0) {
            step1 = 0;
            offset1 = 0;
        } else
            step1 = (f * N / RATE) * (1<<16);
    }
    if (chan == 2) {
        if (f == 0.0) {
            step2 = 0;
            offset2 = 0;
        } else
            step2 = (f * N / RATE) * (1<<16);
    }
    if (chan == 3) {
        if (f == 0.0) {
            step3 = 0;
            offset3 = 0;
        } else
            step3 = (f * N / RATE) * (1<<16);
    }
}