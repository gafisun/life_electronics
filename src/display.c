#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"

/* 7-segment display message buffer, msg[8]

 Note how we've added in the positions as well to make it 
 more SPI-and-DMA-friendly.  So when we configure DMA, we 
 can just copy this buffer to SPI directly and not make 
 any new modifications.

 DMA messes up when we don't align the buffer to 16 bytes,
 so we use the `__attribute__((aligned(16)))` to ensure
 that the buffer is aligned correctly when DMA accesses it.

 We also removed the `static` keyword since the autotest 
 needs to be able to get the address of this array.  
 (Think about why we might need that...)
 `static` variables are not accessible outside the file 
 they are defined in.

 When we update the characters, we will only update [7:0] bits.
*/
uint16_t __attribute__((aligned(16))) msg[8] = {
    (0 << 8) | 0x3F, // seven-segment value of 0
    (1 << 8) | 0x06, // seven-segment value of 1
    (2 << 8) | 0x5B, // seven-segment value of 2
    (3 << 8) | 0x4F, // seven-segment value of 3
    (4 << 8) | 0x66, // seven-segment value of 4
    (5 << 8) | 0x6D, // seven-segment value of 5
    (6 << 8) | 0x7D, // seven-segment value of 6
    (7 << 8) | 0x07, // seven-segment value of 7
};

extern char font[]; // Font mapping for 7-segment display
extern const int SPI_7SEG_SCK; extern const int SPI_7SEG_CSn; extern const int SPI_7SEG_TX;
static int index = 0; // Current index in the message buffer

void display_init_bitbang() {
    sio_hw->gpio_oe_set = (1ul << SPI_7SEG_SCK);
    sio_hw->gpio_clr = (1ul << SPI_7SEG_SCK);
    gpio_set_function(SPI_7SEG_SCK, GPIO_FUNC_SIO);

    sio_hw->gpio_oe_set = (1ul << SPI_7SEG_CSn);
    sio_hw->gpio_set = (1ul << SPI_7SEG_CSn);
    gpio_set_function(SPI_7SEG_CSn, GPIO_FUNC_SIO);

    sio_hw->gpio_oe_set = (1ul << SPI_7SEG_TX);
    sio_hw->gpio_clr = (1ul << SPI_7SEG_TX);
    gpio_set_function(SPI_7SEG_TX, GPIO_FUNC_SIO);
}

void display_bitbang_spi() {
    for (index = 0; index < 8; index++) {
    sio_hw->gpio_clr = (1ul << SPI_7SEG_CSn);
    sleep_us(10);
    for (int i = 15; i >= 0; i--) {
        if (i < 11 && ((msg[index] >> i) & 1)) sio_hw->gpio_set = (1ul << SPI_7SEG_TX); 
        else sio_hw->gpio_clr = (1ul << SPI_7SEG_TX);
        sleep_us(1);
        sio_hw->gpio_set = (1ul << SPI_7SEG_SCK);
        sleep_us(5);
        sio_hw->gpio_clr = (1ul << SPI_7SEG_SCK);
        sleep_us(5);
    }
    sio_hw->gpio_set = (1ul << SPI_7SEG_CSn);
    sleep_us(10);
    }
}

void display_init_spi() {
    spi_init(spi1, 125 * 1000);
    spi_set_format(spi1, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(SPI_7SEG_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_7SEG_CSn, GPIO_FUNC_SPI);
    gpio_set_function(SPI_7SEG_TX, GPIO_FUNC_SPI);
    
}

void display_print() {
    spi_write16_blocking(spi1, msg, 8);
}

void display_init_dma() {
    dma_channel_set_read_addr(0, msg, false);
    dma_channel_set_write_addr(0, &spi1_hw->dr, false);
    dma_channel_set_transfer_count(0, dma_encode_transfer_count_with_self_trigger(8), false);
    uint32_t temp = (1 << 2) //DATA_SIZE
                    | (1 << 4) //INCR_READ
                    | (DREQ_SPI1_TX << 17) //DREQ SRC TREQ_SEL
                    | (1 << 0) //EN
                    | (4 << 8); //RING_SIZE
    dma_hw->ch[0].ctrl_trig = temp;
}

/***************************************************************** */

// We provide you with this function for directly displaying characters.
// This accounts for the decimal point in the 7-segment display, as well as
// the SPI/DMA-friendly format of the message buffer.
void display_char_print(const char message[]) {
    int dp_found = 0; 
    int out_idx = 0;
    for (int i = 0; i < 8 && message[i] != '\0'; i++) {
        if (message[i] == '.') {
            if (dp_found) {
                continue; // Ignore additional decimal points
            }
            if (out_idx > 0) {
                msg[out_idx - 1] |= (1 << 7); // Set decimal point bit
            }
            dp_found = 1;
        } else {
            uint16_t seg = font[(unsigned char)message[i]];
            // Insert position bits at bits 8-10
            seg |= (out_idx & 0x7) << 8;
            msg[out_idx] = seg;
            out_idx++;
        }
    }
    // Clear remaining digits if message shorter than 8
    for (; out_idx < 8; out_idx++) {
        msg[out_idx] = (out_idx << 8); // Only position bits, blank char
    }
}
