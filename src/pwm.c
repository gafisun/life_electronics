#include "pwm.h"

double freq_table[32] = {46.25,55.0,61.74,73.42,82.41,92.5,110.0,123.47,146.83,164.81,185.0,220.0,246.94,293.66,329.63,369.99,440.0,493.88,587.33,659.25,739.99,880.0,987.77,1174.77,1318.51,1479.98,1760.0,1975.53,2349.32,2637.0,2959.96,3520.0};
int col_idx = 0;

int tempo = 750000 / 4; // 90bpm 16th notes
int pitch0 = 261.63;
int pitch1 = 329.63f;
int pitch2 = 392.0f;
int pitch3 = 523.25f;

int envelope_index = 0;

void set_all_freq() {
    int note_num = 0;
    for (int i = 0; i < 32; i++) {
        if(matrix[col_idx][i] == 1) note_num++;
        if (i < 4) set_freq(i, 0.0f);
    }
    for (int i = 0; i < 32; i++) {
        if(matrix[col_idx][i] == 1) {
            if (note_num > 4) {
                if (rand() > RAND_MAX/note_num) {
                    set_freq(note_num, freq_table[i]);
                }
            } else {
                set_freq(note_num, freq_table[i]);
            } 
            note_num--;
        }
    }
    col_idx = col_idx == 32 ? 0 : col_idx+1;
}

void pwm_audio_handler() {
    int slice_one = pwm_gpio_to_slice_num(36);
    pwm_clear_irq(slice_one);

    offset0 += step0;
    offset1 += step1;
    offset2 += step2;
    offset3 += step3;

    if (offset0 >= N << 16) offset0 -= N << 16;
    if (offset1 >= N << 16) offset1 -= N << 16;
    if (offset2 >= N << 16) offset2 -= N << 16;
    if (offset3 >= N << 16) offset3 -= N << 16;

    int samp1 = (wavetable[offset0 >> 16] + wavetable[offset1 >> 16] + wavetable[offset2 >> 16] + wavetable[offset3 >> 16]) * envelope[envelope_index] * play;
    
    samp1 /= 4;
    envelope_index = envelope_index < (M-1) ? envelope_index + 1 : envelope_index;

    samp1 = samp1 * pwm_hw->slice[slice_one].top / (1 << 16);

    pwm_set_chan_level(slice_one, PWM_CHAN_A, samp1);
}

void init_pwm_audio() {
    int gpio1 = 36;
    gpio_init(gpio1);
    gpio_set_function(gpio1, GPIO_FUNC_PWM);

    int slice_one = pwm_gpio_to_slice_num(gpio1);
    float clk_div = 2.34f;

    int period = 150000000.0f / clk_div / RATE;

    pwm_set_clkdiv(slice_one, clk_div);
    pwm_set_wrap(slice_one, period-1);

    int duty = 0;

    pwm_set_chan_level(slice_one, PWM_CHAN_A, duty);

    init_wavetable();
    init_envelope();

    pwm_hw->inte = 1 << slice_one;

    irq_set_exclusive_handler(PWM_IRQ_WRAP_0, pwm_audio_handler);
    irq_set_enabled(PWM_IRQ_WRAP_0, true);

    pwm_set_enabled(slice_one, true);

}

void note_isr() {
    hw_clear_bits(&timer1_hw->intr, 1u << ALARM_ZERO);
    timer1_hw->intr |= (1u << ALARM_ZERO);
    
    if (play) {
        tempo = rate_of_change / 4;
        set_all_freq();
        envelope_index = 0;
    }

    uint64_t target = timer1_hw->timerawl + tempo;
    timer1_hw->alarm[ALARM_ZERO] = (uint32_t) target;
}

void note_init_timer() {
    timer1_hw->inte |= (1u << ALARM_ZERO);
    irq_set_exclusive_handler(ALARM_IRQ0, note_isr);
    irq_set_enabled(ALARM_IRQ0, true);

    uint64_t target0 = timer1_hw->timerawl + tempo;
    timer1_hw->alarm[ALARM_ZERO] = (uint32_t) target0;
}