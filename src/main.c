#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include "hardware/gpio.h"
#include "pico/multicore.h"
#include "hardware/adc.h"
#include "display.h"
#include "adc.h"


#define SIZE 32
#define DRIVE_TIME_MS 10
#define ADC_SAMPLES 8
#define DELAY_MIN_MS 50
#define DELAY_MAX_MS 800

const char keymap[16] = "DCBA#9630852*741";
int cursor_x = SIZE / 2;
int cursor_y = SIZE / 2;


int matrix[SIZE][SIZE]={0};


volatile bool core1_read_access = true;
volatile bool core0_write_access = true;



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


//waits for a key to be pressed, then returns it
char read_key_blocking(){
    gpio_put(6,false);
        gpio_put(7,false);
        gpio_put(8,false);
        gpio_put(9,false);
        sleep_ms(100);
    while(1){

    for(int i = 0; i < 4; i++){ // drive a "row"
        
        gpio_put(6,false);
        gpio_put(7,false);
        gpio_put(8,false);
        gpio_put(9,false);
        gpio_put(i + 6, true);
        sleep_ms(10);
        if(gpio_get(2)){    //based on the "column"

            gpio_put(i+6,false);
            return keymap[4*i]; //return the key based off of the keymap
        }
        else if(gpio_get(3)){

            gpio_put(i+6,false);
            return keymap[4*i + 1];
        }
        else if(gpio_get(4)){

            gpio_put(i+6,false);
            return keymap[4*i + 2];
        }
        else if(gpio_get(5)){

            gpio_put(i+6,false);
            return keymap[4*i + 3];
        }


       gpio_put(i+6,false);
    }
    }
    
}


//draw a frame of the matrix currently stored in matrix[][];
void display_matrix() {
    uint16_t green = display_color565(0,255,0);
    uint16_t red   = display_color565(255,0,0);
    uint16_t white = display_color565(255,255,255);

    while (1) {

        //  wait for read access
        while (!core1_read_access)
            tight_loop_contents();

        //tell core 0 to wait
        core0_write_access = false;

        // Draw the pixels
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

        display_refresh_once(); //refreshes display (somehow)

        core0_write_access = true;
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

    // Wait for core1 to finish reading
    while (!core0_write_access)
        tight_loop_contents();

    // stop core 1 from reading 
    core1_read_access = false;

    //For every cell
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {

            int n = neighbors[row][col];

            if (matrix[row][col]) { // if cell is alive
                //keep it alive if it has 2 or 3 neighbors
                matrix[row][col] = (n == 2 || n == 3);
            } else { //if cell is dead
                //revive if exactly 3 neighbors
                matrix[row][col] = (n == 3);
            }
        }
    }

    // allow core 1 to read again
    core1_read_access = true;
}



int main(){
    stdio_init_all(); // not sure if this is even needed

    // call inits
    
    init_keypad();
    display_init();
    adc_init_pot();
    sleep_ms(50); //make sure inits are done before launching display
    
    //baseline points on the board bc why not
    matrix[16][16] = 1;
    matrix[16][17] = 1;
    matrix[16][15] = 1;

    matrix[9][4] = 1;
    matrix[8][4] = 1;
    matrix[7][4] = 1;
    matrix[9][3] = 1;
    matrix[8][2] = 1;

    //initialize variables
    char key = 0;
    cursor_x = SIZE / 2;
    cursor_y = SIZE / 2;

    multicore_launch_core1(display_matrix);
    // goto keytest;
    while(1){ // repeatedly switch between editor and simulation
        printf("-Start Editor-\n");
        cursor_x = SIZE / 2;
        cursor_y = SIZE / 2;
        key = 0;
        // EDITOR
        do{
            key = read_key_blocking(); //read key from user

            while(!core0_write_access){
                tight_loop_contents();
            }
            core1_read_access = false;
            if(key == '2'){ // move cursor left
                cursor_y = (cursor_y == 0 ? SIZE - 1 : cursor_y - 1);
            }
            else if(key == '6') //move cursor right
            {
                cursor_x = (cursor_x == 0 ? SIZE - 1 : cursor_x - 1);
            }
            else if(key == '5'){ //toggle value
                matrix[cursor_y][cursor_x] = 1 - matrix[cursor_y][cursor_x]; //toggle value at cursor
            }
            else if(key == '4'){ //move cursor up
                cursor_x = (cursor_x == SIZE - 1 ? 0 : cursor_x + 1);
            }
            else if(key == '8'){ //move cursor down
                cursor_y = (cursor_y == SIZE - 1 ? 0 : cursor_y + 1);
            }
            else if(key == 'C'){ //clear
                for(int row = 0; row < SIZE; row++){
                    for(int col = 0; col < SIZE; col++){
                        matrix[row][col] = 0;
                    }
                }
               
            }
            core1_read_access = true;
            sleep_ms(100);
        }while(key != 'D');
        //read key
       
        printf("-=SIMULATION=-\n");
        while(key == 'D'){
            key = read_D();
            sleep_ms(50);
        }
        printf("-Simulation Start-\n");
        cursor_x = -1;
        cursor_y = -1;
        sleep_ms(1000);
        key = 0;
        while(key != 'D'){
            update_matrix();
            key = read_D();
            uint16_t v = adc_read_avg_u12(ADC_SAMPLES);
            uint16_t update_time = adc_scale_u12(v,DELAY_MIN_MS,DELAY_MAX_MS);
            printf("Update Time: %d\n",update_time);
            sleep_ms(update_time);
        }
        printf("-=EDITOR=-\n");
        sleep_ms(100);
        while(key == 'D'){
            key = read_D();
            sleep_ms(50);
        }
    }
    
// keytest:
//     while(1){
//         printf("Key pressed: %c\n", read_key_blocking());
//         sleep_ms(500);
//     }    

    for(;;){} //maintain program control indefinitely
    return 0;
}
    


  