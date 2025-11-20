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
int matrix[SIZE][SIZE]={0};

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
            int val = (matrix[row][col] == 1) ? 255 : 0;
            if(row == cursor_y && col == cursor_x){
                // special cursor indicator
                if(val){
                    display_set_pixel(col,row,display_color565(0,val,0)); // disp green
                }
                else{
                    display_set_pixel(col,row,display_color565(val,0,0)); // disp red
                }
                
            }
            else{
                display_set_pixel(col,row, display_color565(val,val,val)); // disp white if val
            }

        }
    }
    //tells core 1 it can write again
    matrix_is_writeable = true;

    //wait until required based on LED matrix specs
    display_refresh_once();
    sleep_ms(DRIVE_TIME_MS);
}

//update the value of the matrix for the next iteration
void update_matrix(){
    short neighbors[SIZE][SIZE] = {0};

    //find the amt of neighbors for each row
    for(int row = 0; row < SIZE; row++){
        for(int col = 0; col < SIZE; col++){
            if(row != 0){
                neighbors[row][col] += matrix[row-1][col];
            }
            if(col != 0){
                neighbors[row][col] += matrix[row][col-1];
            }
            if(row != SIZE - 1){
                neighbors[row][col] += matrix[row+1][col];
            }
            if(col != SIZE - 1){
                neighbors[row][col] += matrix[row][col+1];
            }
        }
    }

    //make sure core1 is not currently reading from the matrix
    while(!matrix_is_writeable){
        tight_loop_contents();
    }
    //update the matrix values based on the # of neighbors
    matrix_is_readable = false;
    for(int row = 0; row < SIZE; row++){
        for(int col = 0; col < SIZE; col++){
            switch(neighbors[row][col]){
                case 2: //if 2 neighbors, don't change
                matrix[row][col] = matrix[row][col];
                break;
                case 3: //if 3 neighbors, make alive
                matrix[row][col] = 1;
                break;
                default: // if 0,1,4 neighbors, kill
                matrix[row][col] = 0;

            }

        }
    }
    matrix_is_readable = true;
    

}


int main(){
    stdio_init_all(); // not sure if this is even needed

    // call inits
    
    init_keypad();

    char key = 0;

    //TEST DISPLAY 1 MIN
    display_draw_test_pattern();
    sleep_ms(60*1000);


    multicore_launch_core1(display_matrix);
    
    while(1){


    
    cursor_x = SIZE / 2;
    cursor_y = SIZE / 2;
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

            while(!matrix_is_writeable){
                tight_loop_contents();
            }
            matrix_is_readable = false;
            matrix[cursor_y][cursor_x] = 1 - matrix[cursor_y][cursor_x]; //toggle value at cursor
            matrix_is_readable = true;
        }
        else if(key == '6'){
            cursor_x == SIZE - 1 ? 0 : cursor_x + 1;
        }
        else if(key == '8'){
            cursor_y == SIZE - 1 ? 0 : cursor_y + 1;
        }



    } while( key != 'D');

    sleep_ms(100);
    //init interrupt
    init_stop_isr();

    //simulation runner
    cursor_x = -1;
    cursor_y = -1;
    do{

        //update matrix array
        update_matrix();

        //fetch value from ADC
        //int ADC_out = get_val_ADC();
        //sleep_ms(adc_out adjusted);
        sleep_ms(200); //placeholder
    } while(!intr_flag);
    deinit_stop_isr();
    sleep_ms(100);
    intr_flag = 0;
    }

    

    for(;;){} //maintain program control indefinitely
    return 0;
}