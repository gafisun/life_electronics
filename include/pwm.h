#ifndef PWM_H
#define PWM_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "support.h"

#define ALARM_ZERO 0
#define ALARM_IRQ0 timer_hardware_alarm_get_irq_num(timer1_hw, ALARM_ZERO)
#define BASE_FREQ 65.41

extern int rate_of_change;
extern int play;


#endif