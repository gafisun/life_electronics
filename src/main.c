
#include "pwm.h"

//support.c
//void init_wavetable(void);
void set_freq(int chan, float f);

//pwm.c
void init_pwm_audio();
void note_init_timer();

int rate_of_change = 500000;
int play = 1;

int main()
{
    stdio_init_all();
    note_init_timer();
    init_pwm_audio(); 

    set_freq(0, 261.63f); // Set initial frequency to middle c
    set_freq(1, 329.63f);
    set_freq(2, 392.0f);
    set_freq(3, 523.25f);
    
    for(;;) { //testing play toggle
        tight_loop_contents();
        // sleep_ms(1000);
        // play = 1;
        // sleep_ms(10000);
        // play = 0;
    }

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }

    for(;;);
    return 0;
}
