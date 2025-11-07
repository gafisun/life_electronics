#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <stdio.h>
#include "queue.h"

#define ALARM_ZERO 0
#define ALARM_IRQ0 timer_hardware_alarm_get_irq_num(timer_hw, ALARM_ZERO)

#define ALARM_ONE 1
#define ALARM_IRQ1 timer_hardware_alarm_get_irq_num(timer_hw, ALARM_ONE)


// Global column variable
int col = -1;

// Global key state
static bool state[16]; // Are keys pressed/released

// Keymap for the keypad
const char keymap[17] = "DCBA#9630852*741";

void keypad_drive_column();
void keypad_isr();

void keypad_init_pins() {
    for(int i = 2; i < 6; i++) { 
        sio_hw->gpio_oe_set = (1ul << (i + 4)); //outputs to the columns
        sio_hw->gpio_clr = (1ul << (i + 4));
        sio_hw->gpio_oe_clr = (1ul << i); //inputs to the rows
    }
    for(int i= 2; i < 10; i++) {
        hw_write_masked(&pads_bank0_hw->io[i],
                    PADS_BANK0_GPIO0_IE_BITS,
                    PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS
        );
        io_bank0_hw->io[i].ctrl = 5 << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
        hw_clear_bits(&pads_bank0_hw->io[i], PADS_BANK0_GPIO0_ISO_BITS);
    }
}

void keypad_init_timer() {
    //ALARM 0
    timer_hw->inte |= (1u << ALARM_ZERO);
    irq_set_exclusive_handler(ALARM_IRQ0, keypad_drive_column);
    irq_set_enabled(ALARM_IRQ0, true);

    uint64_t target0 = timer_hw->timerawl + 1000000;

    timer_hw->alarm[ALARM_ZERO] = (uint32_t) target0;

    //ALARM 1
    timer_hw->inte |= (1u << ALARM_ONE);
    irq_set_exclusive_handler(ALARM_IRQ1, keypad_isr);
    irq_set_enabled(ALARM_IRQ1, true);

    uint64_t target1 = timer_hw->timerawl + 1100000;

    timer_hw->alarm[ALARM_ONE] = (uint32_t) target1;
}

void keypad_drive_column() {
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_ZERO);
    timer_hw->intr |= (1u << 0);

    col = (col + 1) % 4;
    sio_hw->gpio_set = (1ul << (col+6u));
    sio_hw->gpio_clr = (1ul << (col == 0 ? 9 : col+5));

    uint64_t target = timer_hw->timerawl + 25000;
    timer_hw->alarm[ALARM_ZERO] = (uint32_t) target;
}

uint8_t keypad_read_rows() {
    return (uint8_t) (sio_hw->gpio_in >> 2) & 0xf;
}

void keypad_isr() {
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_ONE);
    timer_hw->intr |= (1u << 1);
    uint8_t rows = keypad_read_rows();

    for (int i = 0; i < 4; i++) {
        if (((rows >> i) & 0b1)) { //Key i*col is currently pressed
            if (state[col * 4 + i] == 0) {
                state[col * 4 + i] = 1;
                key_push((uint16_t) (1 << 8) | keymap[col * 4 + i]);
            }
        } else { //Key i*col is not pressed
            if (state[col * 4 + i] == 1) {
                state[col * 4 + i] = 0;
                key_push((uint16_t) (0 << 8) | keymap[col * 4 + i]);
            }
        }
    }
    
    uint64_t target = timer_hw->timerawl + 25000;
    timer_hw->alarm[ALARM_ONE] = (uint32_t) target;
}