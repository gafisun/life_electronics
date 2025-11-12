#ifndef QUEUE_H
#define QUEUE_H

#include <hardware/timer.h>
#include "pico/stdlib.h"

// Basic queue structure for tracking events
// Tracks 32 events, with each event being {pressed, key}
typedef struct {
    uint16_t q[32];
    uint16_t head;
    uint16_t tail;
} KeyEvents;

// Function definitions are in queue.c

// Global variable to hold key events
extern KeyEvents kev;

// Function to pop a key event from the global `kev` queue
// Blocks until an event is available
uint16_t key_pop();

// Function to push a key event onto the global `kev` queue
// Drops new keys if the queue is full
void key_push(uint16_t value);

#endif