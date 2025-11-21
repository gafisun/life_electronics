
#include "pwm.h"

//support.c
void set_freq(int chan, float f);

//pwm.c
void init_pwm_audio();
void note_init_timer();

int rate_of_change = 750000;
int play = 1;
int matrix[32][32];

int main()
{
    stdio_init_all();
    note_init_timer();
    init_pwm_audio(); 

    set_freq(0, 261.63f); // Set initial frequency to middle c
    set_freq(1, 329.63f);
    set_freq(2, 392.0f);
    set_freq(3, 523.25f);
    srand(11);
    
    for(;;) { //testing play toggle
        for (int i = 0; i < 32; i++) { //random value test
            for (int j = 0; j < 32; j++) {
                matrix[i][j] = rand() % 29;
            }
        }
        sleep_us(rate_of_change);
    }

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }

    for(;;);
    return 0;
}
