#include "pwm.h"

void pwm_audio_handler();
void note_isr();
void init_pwm_audio();
void note_init_timer();


int col_d[32] = {0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0}; //state of current colum, (could use an array or an int)
int col_num = 0;
int tempo = 1000000 / 4; // 90bpm 16th notes
int pitch0 = 261.63;
int pitch1 = 329.63f;
int pitch2 = 392.0f;
int pitch3 = 523.25f;
int volume = 1;

int envelope_index = 0;


void pwm_audio_handler() {
    int slice_one = pwm_gpio_to_slice_num(36);
    //int slice_two = pwm_gpio_to_slice_num(38);
    pwm_clear_irq(slice_one);
    //pwm_clear_irq(slice_two);

    offset0 += step0;
    offset1 += step1;
    offset2 += step2;
    offset3 += step3;

    if (offset0 >= N << 16) offset0 -= N << 16;
    if (offset1 >= N << 16) offset1 -= N << 16;
    if (offset2 >= N << 16) offset2 -= N << 16;
    if (offset3 >= N << 16) offset3 -= N << 16;

    int samp1 = (wavetable[offset0 >> 16] + wavetable[offset1 >> 16] + wavetable[offset2 >> 16] + wavetable[offset3 >> 16]) * envelope[envelope_index] * play * volume;
    //int samp2 = (wavetable[offset2 >> 16] + wavetable[offset3 >> 16]) * envelope[envelope_index] * play * volume;
    
    samp1 /= 4;
    //samp2 /= 2;

    envelope_index = envelope_index < (M-1) ? envelope_index + 1 : envelope_index;

    samp1 = samp1 * pwm_hw->slice[slice_one].top / (1 << 16);
    //samp2 = samp2 * pwm_hw->slice[slice_two].top / (1 << 16);

    pwm_set_chan_level(slice_one, PWM_CHAN_A, samp1);
    //pwm_set_chan_level(slice_two, PWM_CHAN_A, samp2);
}

void init_pwm_audio() {
    int gpio1 = 36;
    //int gpio2 = 38;
    gpio_init(gpio1);
    gpio_set_function(gpio1, GPIO_FUNC_PWM);
    //gpio_init(gpio2);
    //gpio_set_function(gpio2, GPIO_FUNC_PWM);

    int slice_one = pwm_gpio_to_slice_num(gpio1);
    //int slice_two = pwm_gpio_to_slice_num(gpio2);
    float clk_div = 2.34f;

    int period = 150000000.0f / clk_div / RATE;

    pwm_set_clkdiv(slice_one, clk_div);
    pwm_set_wrap(slice_one, period-1);
    //pwm_set_clkdiv(slice_two, clk_div);
    //pwm_set_wrap(slice_two, period-1);

    int duty = 0;

    pwm_set_chan_level(slice_one, PWM_CHAN_A, duty);
    //pwm_set_chan_level(slice_two, PWM_CHAN_A, duty);

    init_wavetable();
    init_envelope();

    pwm_hw->inte = 1 << slice_one /*| 1 << slice_two*/;

    irq_set_exclusive_handler(PWM_IRQ_WRAP_0, pwm_audio_handler);
    irq_set_enabled(PWM_IRQ_WRAP_0, true);

    pwm_set_enabled(slice_one, true);
    //pwm_set_enabled(slice_two, true);

}

void note_isr() {
    hw_clear_bits(&timer1_hw->intr, 1u << ALARM_ZERO);
    timer1_hw->intr |= (1u << ALARM_ZERO);
    
    if (rand() >  RAND_MAX/8) {
    if (play) {
        tempo = rate_of_change / 4;
        set_freq(0, pitch0);
        set_freq(1, pitch1);
        set_freq(2, pitch2);
        set_freq(3, pitch3);
        pitch0 *= 1.06f;
        pitch1 *= 1.06f;
        pitch2 *= 1.06f;
        pitch3 *= 1.06f;
        if (pitch0 > 520) {
            pitch0 = 261.63;
            pitch1 = 329.63f;
            pitch2 = 392.0f;
            pitch3 = 523.25f;
        }
        envelope_index = 0;
        }
    }

    uint64_t target = timer1_hw->timerawl + tempo;
    timer1_hw->alarm[ALARM_ZERO] = (uint32_t) target;
}

void note_init_timer() {
    timer1_hw->inte |= (1u << ALARM_ZERO);
    irq_set_exclusive_handler(ALARM_IRQ0, note_isr);
    irq_set_enabled(ALARM_IRQ0, true);

    uint64_t target0 = timer1_hw->timerawl + tempo; //120bpm 1/16
    timer1_hw->alarm[ALARM_ZERO] = (uint32_t) target0;
}