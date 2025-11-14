#include <stdio.h>
#include <stdlib.h>
#include "hardware/gpio.h"
#include "pico/multicore.h"

//#include "display.h"
//#include "display.c"


#define SIZE 32
#define DRIVE_TIME_MS 16
const char keymap[16] = "DCBA#9630852*741";
char cursor_x = SIZE / 2;
char cursor_y = SIZE / 2;
//definitions
/*
void init_keypad();
void init_adc();
void init_pwm();
void init_matrix();
*/ // something like these are needed

bool intr_flag = false;
bool matrix_is_readable = true;
bool matrix_is_writeable = true;



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
    intr_flag = false;
    gpio_set_irq_enabled(2,GPIO_IRQ_EDGE_RISE,false);
    gpio_put(2,false);
}


char read_key(){

    gpio_put(6,false);
    gpio_put(7,false);
    gpio_put(8,false);
    gpio_put(9,false);
    for(int i = 0; i < 4; i++){
        gpio_put(i + 6, true);

        if(gpio_get(2)){
            gpio_put(i+6,false);
            return keymap[4*i];
        }
        else if(gpio_get(3)){
            gpio_put(i+6,false);
            return keymap[4*i+1];
        }
        else if(gpio_get(4)){
            gpio_put(i+6,false);
            return keymap[4*i+2];
        }
        else if(gpio_get(5)){
            gpio_put(i+6,false);
            return keymap[4*i+3];
        }


       gpio_put(i+6,false);
    }
    
    return 0;
}




void display_matrix(){

    //waits until core 0 is done writing into the matrix
    while(!matrix_is_readable){
        tight_loop_contents();
    } 

    //tells core 1 not to write to the matrix
    matrix_is_writeable = false;

    //drive the LEDS 
    for(int row = 0; row < SIZE; row++){
        for(int col = 0; col < SIZE; col++){
            if(row == cursor_y && col == cursor_x){
                // special cursor indicator
            }
            else{
                //drive_led(row,col,matrix[row][col]);
            }

        }
    }
    //tells core 1 it can write again
    matrix_is_writeable = true;

    //wait until required based on LED matrix specs
    sleep_ms(DRIVE_TIME_MS);
}


void update_matrix(){


}


int main(){
    stdio_init_all(); // not sure if this is even needed

    // call inits
    
    init_keypad();

    char key = 0;

    multicore_launch_core1(display_matrix);
    
    while(1){


    

    //editor
    do
    {
        sleep_ms(40);
        //read key
        key = read_key();
        //set cursor indication
        if(key == '2'){
            cursor_y == 0 ? SIZE - 1 : cursor_y - 1;
        }
        else if(key == '4')
        {
            cursor_x == 0 ? SIZE - 1 : cursor_x - 1;
        }
        else if(key == '5'){
            //toggle pixel at cursor
        }
        else if(key == '6'){
            cursor_x == SIZE - 1 ? 0 : cursor_x + 1;
        }
        else if(key == '8'){
            cursor_y == SIZE - 1 ? 0 : cursor_y + 1;
        }



    } while( key != 'D');

    //init interrupt
    init_stop_isr();

    //simulation runner
    do{

        //update matrix array
        update_matrix();

        //fetch value from ADC

        //sleep_ms(adc_out adjusted);

    } while(!intr_flag);
    deinit_stop_isr();
    sleep_ms(100);

    }

    

    for(;;){} //maintain program control indefinitely
    return 0;
}