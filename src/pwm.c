#define ALARM_ZERO 0
#define ALARM_IRQ0 timer_hardware_alarm_get_irq_num(timer1_hw, ALARM_ZERO)
#define BASE_FREQ 65.41


int col_d[32] = {0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0}; 
int col_num = 0;
int rate_of_change = 500000; // 120/s in microseconds
int tempo = rate_of_change / 4; // 16th notes

int envelope = 3000;


void pwm_audio_handler() {
    int slice_num = pwm_gpio_to_slice_num(36);
    pwm_clear_irq(slice_num);

    offset0 += step0;
    offset1 += step1;

    if (offset0 >= N << 16) offset0 -= N << 16;
    if (offset1 >= N << 16) offset1 -= N << 16;

    int samp = wavetable[offset0 >> 16] + wavetable[offset1 >> 16];

    samp /= 2;

    samp = samp * envelope / 3000;
    envelope -= 1;

    samp = samp * pwm_hw->slice[slice_num].top / (1 << 16);

    pwm_set_chan_level(slice_num, PWM_CHAN_A, samp);
}


void note_init_timer() {
    timer1_hw->inte |= (1u << ALARM_ZERO);
    irq_set_exclusive_handler(ALARM_IRQ0, note_isr);
    irq_set_enabled(ALARM_IRQ0, true);

    uint64_t target0 = timer1_hw->timerawl + tempo; //120bpm 1/16
    timer1_hw->alarm[ALARM_ZERO] = (uint32_t) target0;
}
void note_isr() {
    hw_clear_bits(&timer1_hw->intr, 1u << ALARM_ZERO);
    timer1_hw->intr |= (1u << ALARM_ZERO);

    set_freq(0, pow(2.0, (sum_col(col_d) / 12.0)) + BASE_FREQ);
    envelope = 3000;

    uint64_t target = timer1_hw->timerawl + tempo;
    timer1_hw->alarm[ALARM_ZERO] = (uint32_t) target;
}

double sum_col(int col_data[]) {
    double sum = 0;
    for (int i = 0; i < 32; i++) {
        if (col_data[i]) {
            sum += 1;
        }
    }
    return sum;
}

void init_pwm_audio() {
    int gpio = 36;
    gpio_init(gpio);
    gpio_set_function(gpio, GPIO_FUNC_PWM);

    int slice_num = pwm_gpio_to_slice_num(gpio);

    int period = 1000000 / RATE;

    pwm_set_clkdiv(slice_num, 150.0f);
    pwm_set_wrap(slice_num, period-1);

    int duty = 0;

    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(gpio), duty);

    init_wavetable();

    pwm_hw->inte = 1 << slice_num;

    irq_set_exclusive_handler(PWM_IRQ_WRAP_0, pwm_audio_handler);
    irq_set_enabled(PWM_IRQ_WRAP_0, true);

    pwm_set_enabled(slice_num, true);

}