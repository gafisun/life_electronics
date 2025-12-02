#ifndef PWM_H
#define PWM_H

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "support.h"

#define ALARM_ZERO 0
#define ALARM_IRQ0 timer_hardware_alarm_get_irq_num(timer1_hw, ALARM_ZERO)

void pwm_audio_handler();
void note_play();
void init_pwm_audio();
void set_all_freq();

extern int matrix[32][32];


#endif