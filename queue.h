#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include "task1.h"
#include "mutex.h"

#define QUEUE_SIZE 50

typedef struct queue {
    uint32_t buffer[QUEUE_SIZE];
    uint32_t head;              
    uint32_t tail;              
    uint32_t count;             
    mutex_t lock;               
} queue_t;


void queue_init(queue_t *q);
int queue_send(queue_t *q, uint32_t data);
int queue_receive(queue_t *q, uint32_t *data);

#endif