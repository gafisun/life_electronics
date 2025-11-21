#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include "hardware/gpio.h"
#include "pico/multicore.h"

#include "display.h"



#define SIZE 32
#define DRIVE_TIME_MS 10
#define ADC_SAMPLES 8
#define DELAY_MIN_MS 30
#define DELAY_MAX_MS 800

const char keymap[16] = "DCBA#9630852*741";
int cursor_x = SIZE / 2;
int cursor_y = SIZE / 2;



//definitions
/*
void init_keypad();
void init_adc();
void init_pwm();
void init_matrix();
*/ // something like these are needed
int matrix[SIZE][SIZE]={0};

volatile bool intr_flag = false;
volatile bool matrix_is_readable = true;
volatile bool matrix_is_writeable = true;



void init_keypad(){
    
    //set 6-9 as outputs (so we can drive them)
    for(int i = 6; i <=9; i++)
    {
        gpio_init(i);
        gpio_set_function(i,GPIO_FUNC_SIO);
        gpio_set_dir(i,true);
        gpio_put(i,false);
    }

   
    //set 2-5 as inputs (so we can read them)
    for(int i = 2; i <= 5; i++){
        gpio_init(i);
        gpio_set_function(i,GPIO_FUNC_SIO);
        gpio_set_dir(i,false);
        gpio_pull_down(i);
    }
}




char read_D() {
    
    gpio_put(6, true);
    gpio_put(7, false);
    gpio_put(8, false);
    gpio_put(9, false);
    sleep_ms(10);

    if(gpio_get(2)){
        sleep_ms(50);
        if(gpio_get(2)){
            gpio_put(6,false);
            return 'D';
        }
    }

    
    
    return 0;
}



char read_key_blocking(){
    gpio_put(6,false);
        gpio_put(7,false);
        gpio_put(8,false);
        gpio_put(9,false);
        sleep_ms(100);
    while(1){

    for(int i = 0; i < 4; i++){
        
        gpio_put(6,false);
        gpio_put(7,false);
        gpio_put(8,false);
        gpio_put(9,false);
        gpio_put(i + 6, true);
        sleep_ms(10);
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
    }
    
}
/*
char read_key_blocking() {
    while (1) {
        gpio_put(6, false);
        gpio_put(7, false);
        gpio_put(8, false);
        gpio_put(9, false);
        
        for (int i = 0; i < 4; i++) {
            gpio_put(i + 6, true); // Drive the row

            // Check columns
            if (gpio_get(2)) { // Column 2
                sleep_ms(50);  // Wait for key state to stabilize
                if (gpio_get(2)) {  // Recheck to confirm key press
                    gpio_put(i + 6, false);  // Stop driving this row
                    return keymap[4 * i];  // Return key
                }
            } else if (gpio_get(3)) { // Column 3
                sleep_ms(50);
                if (gpio_get(3)) {
                    gpio_put(i + 6, false);
                    return keymap[4 * i + 1];
                }
            } else if (gpio_get(4)) { // Column 4
                sleep_ms(50);
                if (gpio_get(4)) {
                    gpio_put(i + 6, false);
                    return keymap[4 * i + 2];
                }
            } else if (gpio_get(5)) { // Column 5
                sleep_ms(50);
                if (gpio_get(5)) {
                    gpio_put(i + 6, false);
                    return keymap[4 * i + 3];
                }
            }

            gpio_put(i + 6, false); // Reset row after checking
        }
    }
}
*/



void display_matrix() {
    uint16_t green = display_color565(0,255,0);
    uint16_t red   = display_color565(255,0,0);
    uint16_t white = display_color565(255,255,255);

    while (1) {

        // Wait until update_matrix() finishes writing
        while (!matrix_is_readable)
            tight_loop_contents();

        // LOCK matrix (update core cannot write during drawing)
        matrix_is_writeable = false;

        // Draw one full frame
        for (int row = 0; row < SIZE; row++) {
            for (int col = 0; col < SIZE; col++) {

                uint16_t color;
                if (row == cursor_y && col == cursor_x)
                    color = matrix[row][col] ? green : red;
                else
                    color = matrix[row][col] ? white : 0;

                display_set_pixel(row, col, color);
            }
        }

        display_refresh_once();

        // can now run update_matrix
        matrix_is_writeable = true;

       
        //frame delay
        sleep_ms(DRIVE_TIME_MS); 
    }
}


//update the value of the matrix for the next iteration
void update_matrix() {
    short neighbors[SIZE][SIZE] = {0};

    // Compute neighbors 
    for (int row = 0; row < SIZE; row++) {
        int row_abv   = (row == 0 ? SIZE - 1 : row - 1);
        int row_below = (row == SIZE - 1 ? 0: row + 1);

        for (int col = 0; col < SIZE; col++) {
            int col_left  = (col == 0 ? SIZE - 1 : col - 1);
            int col_right = (col == SIZE - 1 ? 0: col + 1);

            neighbors[row][col] = matrix[row_abv][col_left]  + matrix[row_abv][col] + matrix[row_abv][col_right];
            neighbors[row][col] += matrix[row][col_left] + matrix[row][col_right];
            neighbors[row][col] += matrix[row_below][col_left] + matrix[row_below][col] + matrix[row_below][col_right];
        }
    }

    // Wait for display core to finish reading
    while (!matrix_is_writeable)
        tight_loop_contents();

    // LOCK matrix (display may NOT read during update)
    matrix_is_readable = false;

    // Apply rules
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {

            int n = neighbors[row][col];

            if (matrix[row][col]) {
                // Alive cell
                matrix[row][col] = (n == 2 || n == 3);
            } else {
                // Dead cell
                matrix[row][col] = (n == 3);
            }
        }
    }

    // Allow display core to read the new frame
    matrix_is_readable = true;
}



int main(){
    stdio_init_all(); // not sure if this is even needed

    // call inits
    
    init_keypad();
    display_init();
    sleep_ms(50);
    char key = 0;

    //TEST DISPLAY 1 MIN
    //display_draw_test_pattern();
    //sleep_ms(60*1000);

    
    matrix[16][16] = 1;
    matrix[16][17] = 1;
    matrix[16][15] = 1;

    matrix[9][4] = 1;
    matrix[8][4] = 1;
    matrix[7][4] = 1;
    matrix[9][3] = 1;
    matrix[8][2] = 1;

    cursor_x = SIZE / 2;
    cursor_y = SIZE / 2;

    multicore_launch_core1(display_matrix);
    //sleep_ms(5000);
    // goto keytest;
    while(1){
        printf("Start EDITOR\n");
        cursor_x = SIZE / 2;
        cursor_y = SIZE / 2;
        key = 0;
        do{
            key = read_key_blocking();
            if(key == '2'){
                cursor_y = (cursor_y == 0 ? SIZE - 1 : cursor_y - 1);
            }
            else if(key == '6')
            {
                cursor_x = (cursor_x == 0 ? SIZE - 1 : cursor_x - 1);
            }
            else if(key == '5'){

                while(!matrix_is_writeable){
                    tight_loop_contents();
                }
                matrix_is_readable = false;
                matrix[(int)cursor_y][(int)cursor_x] = 1 - matrix[(int)cursor_y][(int)cursor_x]; //toggle value at cursor
                matrix_is_readable = true;
            }
            else if(key == '4'){
                cursor_x = (cursor_x == SIZE - 1 ? 0 : cursor_x + 1);
            }
            else if(key == '8'){
                cursor_y = (cursor_y == SIZE - 1 ? 0 : cursor_y + 1);
            }
            sleep_ms(300);
        }while(key != 'D');
        //read key
       
        printf("START SIMULATION\n");
        cursor_x = -1;
        cursor_y = -1;
        //init_stop_isr();
        sleep_ms(1000);
        key = 0;
        intr_flag = false;
        while(key != 'D'){
            update_matrix();
            key = read_D();
           sleep_ms(200);
        }
        //sleep_ms(1000);
        //deinit_stop_isr();
        intr_flag = false;
        while(gpio_get(2)){
            printf("Waiting\n");
            sleep_ms(300);
        }
    }
    
    // while(1){


    
    // cursor_x = SIZE / 2;
    // cursor_y = SIZE / 2;
    // //editor
    // do
    // {
    //     sleep_ms(40);
    //     //read key
    //     key = read_key();
    //     //set cursor indication
    //     if(key == '2'){
    //         cursor_y = (cursor_y == 0 ? SIZE - 1 : cursor_y - 1);
    //     }
    //     else if(key == '4')
    //     {
    //         cursor_x = (cursor_x == 0 ? SIZE - 1 : cursor_x - 1);
    //     }
    //     else if(key == '5'){

    //         while(!matrix_is_writeable){
    //             tight_loop_contents();
    //         }
    //         matrix_is_readable = false;
    //         matrix[(int)cursor_y][(int)cursor_x] = 1 - matrix[(int)cursor_y][(int)cursor_x]; //toggle value at cursor
    //         matrix_is_readable = true;
    //     }
    //     else if(key == '6'){
    //         cursor_x = (cursor_x == SIZE - 1 ? 0 : cursor_x + 1);
    //     }
    //     else if(key == '8'){
    //         cursor_y = (cursor_y == SIZE - 1 ? 0 : cursor_y + 1);
    //     }



    // } while( key != 'D');

    // sleep_ms(100);
    // //init interrupt
    // init_stop_isr();

    // //simulation runner
    // cursor_x = -1;
    // cursor_y = -1;
    // do{

    //     //update matrix array
    //     update_matrix();

    //     uint16_t v = 3;//adc_read_avg_u12(ADC_SAMPLES);
    //     uint16_t delay_ms =  50;//adc_scale_u12(v,DELAY_MIN_MS,DELAY_MAX_MS);
    //     sleep_ms(delay_ms);
    // } while(!intr_flag);
    // deinit_stop_isr();
    // sleep_ms(100);
    // intr_flag = 0;
    // }
keytest:
    while(1){
        printf("Key pressed: %c\n", read_key_blocking());
        gpio_pull_down(2);
        sleep_ms(500);
    }    

    for(;;){} //maintain program control indefinitely
    return 0;
}
    


  