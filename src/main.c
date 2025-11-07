#include <stdio.h>
#include <stdlib.h>
#include "hardware/gpio.h"
#include "pico/multicore.h"

//definitions
/*
void init_keypad();
void init_adc();
void init_pwm();
void init_matrix();
*/ // something like these are needed

bool intr_flag;
bool matrix_is_readable;
bool matrix_is_writable;


void init_keypad(){
    
    //set 2-5 as outputs (so we can drive them)
    for(int i = 2; i <=5; i++)
    {
        gpio_set_dir(i,true);
        gpio_put(i,false);
        gpio_set_function(i,GPIO_FUNC_SIO);
    }
    
    //set 6-9 as inputs (so we can read them)
    for(int i = 6; i <= 9; i++){
        gpio_init(i);
    }

}


void stop_isr(){
    gpio_acknowledge_irq(6,GPIO_IRQ_EDGE_RISE);
    intr_flag = true;
}


//set up the code to wait for rising edge
void init_stop_isr(){
    gpio_add_raw_irq_handler(6,stop_isr);
    gpio_set_irq_enabled(6,GPIO_IRQ_EDGE_RISE,true);
    irq_set_enabled(IO_IRQ_BANK0,true);

}

void deinit_stop_isr(){
    //
    gpio_set_irq_enabled(6,GPIO_IRQ_EDGE_RISE,false);
}

/*
void update_display(){

    while(!read_mat_enable){
        tight_loop_contents();
    }
    write_mat_enable = false;

    for(int row = 0; row < 32; row++){
        for(int col = 0; col < 32; col++){
            drive_led(row,col,matrix[row][col]);
        }
    }

    sleep_ms(drive_time_ms);
}*/


int main(){
    stdio_init_all(); // not sure if this is even needed

    // call inits
    /*
    init_keypad();
    
    while(1){


    multicore_launch_core1(update_display);

    //editor
    do{

        //read key

        //set cursor indication


        //perform instruction


    } while( key_pressed != EXIT_KEY);

    //init interrupt

    //simulation runner
    do{




    } while(!intr_flag)


    }

    */
}