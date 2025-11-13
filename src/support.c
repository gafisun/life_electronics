#include "support.h"


int step0 = 0;
int offset0 = 0;

//wave parameters
double A = 1.0;
double beta = 4.0; // High modulation depth
double K = 1.4142; // Inharmonic ratio (sqrt(2))


void init_wavetable(void) {
    for(int i=0; i < N; i++) {
        double phase = 2.0 * M_PI * i / N;
        double wave_fm = A * sin(phase + (beta * sin(K * phase)));;
        wavetable[i] = (16383 * (wave_fm)) + 16384;
    }
}

//Better Envelopw
// const double ATTACK_CUTOFF = .1 * N;
// const double ATTACK_MAX = 1;
// const double DIP_CUTTOFF = .2 * N;
// const double DIP_DOWN = .2;
// const double HOLD_CUTTOFF = .6 * N;
// const double HOLD_LEVEL = DIP_MIN;
// const double DECAY_CUTOFF = N;



// void init_envelope(void) {
//     for(int i=0; i < N; i++) {
//         double wave_en = 0;
//         if (i < (int)ATTACK_CUTTOFF) {
//             wave_en = ATTACK_MAX * (i/ATTACK_CUTOFF);
//         } else if (i < (int)DIP_CUTTOFF) {;
//             wave_en = ATTACK_MAX - DIP_DOWN * (i-ATTACK_CUTOFF/i-DIP_CUTTOFF)
//         }
//     }
// }


void set_freq(float f) {
    if (f == 0.0) {
        step0 = 0;
        offset0 = 0;
    } else {
        step0 = (f * N / RATE) * (1<<16);
    }
}