#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "queue.h"
#include "support.h"
#include "hardware/irq.h"

const bool USING_LCD = true; // Set to true if using LCD, false if using OLED, for check_wiring.
const int SEG7_DMA_CHANNEL = 0;  // Set to the correct DMA channel for the 7-segment display for Step 4 autotest.

const int SPI_7SEG_SCK = 14; // Replace with your SCK pin number for the 7-segment display
const int SPI_7SEG_CSn = 13; // Replace with your CSn pin number for the 7-segment display
const int SPI_7SEG_TX = 15; // Replace with your TX pin number for the 7-segment display

static int duty_cycle = 0;
static int dir = 0;
static int color = 0;

void display_init_spi();
void display_init_dma();
void display_char_print(const char message[]);
void keypad_init_pins();
void keypad_init_timer();
void init_wavetable(void);
void set_freq(int chan, float f);
extern KeyEvents kev;


void init_pwm_static(uint32_t period, uint32_t duty_cycle) {
    for (int i = 37; i <= 39; i++) {
        sio_hw->gpio_oe_set = (1ul << i); //10B, 11A, 11B
        sio_hw->gpio_clr = (1ul << i);
        hw_write_masked(&pads_bank0_hw->io[i],
                    PADS_BANK0_GPIO0_IE_BITS,
                    PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS
        );
        io_bank0_hw->io[i].ctrl = GPIO_FUNC_PWM << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
        hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_ISO_BITS);

        if (i % 2) {
            uint slice_num = pwm_gpio_to_slice_num(i);
            pwm_set_clkdiv(slice_num, 150);
            pwm_set_wrap(slice_num, period-1);
            pwm_set_chan_level(slice_num, PWM_CHAN_A, duty_cycle);
            pwm_set_chan_level(slice_num, PWM_CHAN_B, duty_cycle);
            pwm_set_enabled(slice_num, true);
        }
    }
}


void pwm_breathing() {
    pwm_clear_irq(pwm_gpio_to_slice_num(37));
    
    if (dir == 0 & duty_cycle == 100) {
        color = (color +1) % 3;
        dir = 1;
    } else if (duty_cycle == 0 && dir == 1) {
        dir = 0;
    } 

    if (dir == 0) {
        duty_cycle += 1;
    } else {
        duty_cycle -= 1;
    }

    if (color == 0) {
        pwm_set_chan_level(pwm_gpio_to_slice_num(37), PWM_CHAN_B,   duty_cycle * pwm_hw->slice[pwm_gpio_to_slice_num(37)].top / 100);
    } else if (color == 1) {
        pwm_set_chan_level(pwm_gpio_to_slice_num(38), PWM_CHAN_A,  duty_cycle * pwm_hw->slice[pwm_gpio_to_slice_num(38)].top / 100);
    } else {
        pwm_set_chan_level(pwm_gpio_to_slice_num(38), PWM_CHAN_B,  duty_cycle * pwm_hw->slice[pwm_gpio_to_slice_num(38)].top / 100);
    }
}

void init_pwm_irq() {
    int slice_num37 = pwm_gpio_to_slice_num(37);
    pwm_hw->inte = 1 << slice_num37;

    irq_set_exclusive_handler(PWM_IRQ_WRAP_0, pwm_breathing);
    irq_set_enabled(PWM_IRQ_WRAP_0, true);

    int current_period =  pwm_hw->slice[pwm_gpio_to_slice_num(37)].top;
    duty_cycle = 100;
    dir = 1;

    pwm_set_chan_level(pwm_gpio_to_slice_num(37), PWM_CHAN_B, current_period);    
    display_init_spi();
    display_init_dma();
    pwm_set_chan_level(pwm_gpio_to_slice_num(38), PWM_CHAN_A, current_period);
    pwm_set_chan_level(pwm_gpio_to_slice_num(39), PWM_CHAN_B, current_period);

}

void pwm_audio_handler() {
    int slice_num = pwm_gpio_to_slice_num(36);
    pwm_clear_irq(slice_num);

    offset0 += step0;
    offset1 += step1;

    if (offset0 >= N << 16) offset0 -= N << 16;
    if (offset1 >= N << 16) offset1 -= N << 16;

    int samp = wavetable[offset0 >> 16] + wavetable[offset1 >> 16];

    samp /= 2;

    samp = samp * pwm_hw->slice[slice_num].top / (1 << 16);

    pwm_set_chan_level(slice_num, PWM_CHAN_A, samp);

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


int main()
{
    // Configures our microcontroller to 
    // communicate over UART through the TX/RX pins
    stdio_init_all();

    
    keypad_init_pins();
    keypad_init_timer();
    display_init_spi();
    display_init_dma();

    char freq_buf[9] = {0};
    int pos = 0;
    bool decimal_entered = false;
    int decimal_pos = 0;
    int current_channel = 0;

    init_pwm_audio(); 
    // set_freq(0, 440.0f); // Set initial frequency to 440 Hz (A4 note)
    // set_freq(1, 0.0f); // Turn off channel 1 initially
    // set_freq(0, 261.626f);
    // set_freq(1, 329.628f);

    set_freq(0, 440.0f); // Set initial frequency for channel 0
    display_char_print(" 440.000 ");

    for(;;) {
        uint16_t keyevent = key_pop();

        if (keyevent & 0x100) {
            char key = keyevent & 0xFF;
            if (key == 'A') {
                current_channel = 0;
                pos = 0;
                freq_buf[0] = '\0';
                decimal_entered = false;
                decimal_pos = 0;
                display_char_print("         ");
            } else if (key == 'B') {
                current_channel = 1;
                pos = 0;
                freq_buf[0] = '\0';
                decimal_entered = false;
                decimal_pos = 0;
                display_char_print("         ");
            } else if (key >= '0' && key <= '9') {
                if (pos == 0) {
                    snprintf(freq_buf, sizeof(freq_buf), "        "); // Clear buffer on first digit
                    display_char_print(freq_buf);
                }
                if (pos < 8) {
                    freq_buf[pos++] = key;
                    freq_buf[pos] = '\0';
                    display_char_print(freq_buf);
                    if (decimal_entered) decimal_pos++;
                }
                } else if (key == '*') {
                if (!decimal_entered && pos < 7) {
                    freq_buf[pos++] = '.';
                    freq_buf[pos] = '\0';
                    display_char_print(freq_buf);
                    decimal_entered = true;
                    decimal_pos = 0;
                }
                } else if (key == '#') {
                float freq = 0.0f;
                if (decimal_entered) {
                    freq = strtof(freq_buf, NULL);
                } else {
                    freq = (float)atoi(freq_buf);
                }
                set_freq(current_channel, freq);
                snprintf(freq_buf, sizeof(freq_buf), "%8.3f", freq);
                display_char_print(freq_buf);
                pos = 0;
                freq_buf[0] = '\0';
                decimal_entered = false;
                decimal_pos = 0;
            } else {
                // Reset on any other key
                pos = 0;
                freq_buf[0] = '\0';
                decimal_entered = false;
                decimal_pos = 0;
                display_char_print("        ");
            }
        }
    }

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }

    for(;;);
    return 0;
}
