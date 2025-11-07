#include <stdio.h>
#include <stdlib.h>
#include "hardware/gpio.h"
#include "pico/multicore.h"

#define SIZE 32
const char keymap[16] = "DCBA#9630852*741";
int col;
//definitions
/*
void init_keypad();
void init_adc();
void init_pwm();
void init_matrix();
*/ // something like these are needed

bool intr_flag;
bool matrix_is_readable = true;
bool matrix_is_writable = true;



void init_keypad(){
    
    //set 6-9 as outputs (so we can drive them)
    for(int i = 6; i <=9; i++)
    {
        gpio_set_dir(i,true);
        gpio_put(i,false);
        gpio_set_function(i,GPIO_FUNC_SIO);
    }
    
    //set 2-5 as inputs (so we can read them)
    for(int i = 2; i <= 5; i++){
        gpio_init(i);
    }

}

    // when interrupted, we set the intr_flag to be true
void stop_isr(){
    gpio_acknowledge_irq(2,GPIO_IRQ_EDGE_RISE);
    intr_flag = true;
}


//set up the code to wait for rising edge on pin 2 (key D)
void init_stop_isr(){
    gpio_add_raw_irq_handler(2,stop_isr);
    gpio_set_irq_enabled(2,GPIO_IRQ_EDGE_RISE,true);
    irq_set_enabled(IO_IRQ_BANK0,true);
    gpio_put(2,true);

}

void deinit_stop_isr(){

    gpio_set_irq_enabled(2,GPIO_IRQ_EDGE_RISE,false);
    gpio_put(2,false);
}


/*
void display_matrix(){

    //waits until core 0 is done writing into the matrix
    while(!matrix_is_readable){
        tight_loop_contents();
    } 

    //tells core 1 not to write to the matrix
    write_mat_enable = false;

    //drive the LEDS 
    for(int row = 0; row < SIZE; row++){
        for(int col = 0; col < SIZE; col++){
            drive_led(row,col,matrix[row][col]);
        }
    }
    //tells core 1 it can write again
    write_mat_enable = true;

    //wait until required based on LED matrix specs
    sleep_ms(drive_time_ms);
}*/


int main(){
    stdio_init_all(); // not sure if this is even needed

    // call inits
    /*
    init_keypad();

    char key = 0;

    multicore_launch_core1(display_matrix);
    
    while(1){


    

    //editor
    do
    {

        //read key
        key = read_key_blocking();
        //set cursor indication
        if(key == '2'){
        
        }
        else if(key == '4')
        {

        }
        else if(key == '5')


        //perform instruction


    } while( key_pressed != EXIT_KEY);

    //init interrupt

    //simulation runner
    do{




    } while(!intr_flag)
    deinit_stop_isr();


    }

    */

    for(;;){} //maintain program control indefinitely
    return 0;
}