
#include "pwm.h"

//support.c
void init_wavetable(void);
void set_freq(float f);

//pwm.c
void init_pwm_audio();
void note_init_timer();

int rate_of_change = 500000;
int play = 0;

int main()
{
    stdio_init_all();
    note_init_timer();
    init_pwm_audio(); 

    set_freq(440.0f); // Set initial frequency
    
    for(;;) {
        tight_loop_contents();
    }

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }

    for(;;);
    return 0;
}
