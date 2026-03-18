#include "queue.h"

void queue_init(queue_t *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    mutex_init(&q->lock, 1);
}

static bool is_full(queue_t *q) {
    return (q->count >= QUEUE_SIZE);
}

static bool is_empty(queue_t *q) {
    return (q->count == 0);
}

int queue_send(queue_t *q, uint32_t data) {
    int status = -1;
    
    mutex_lock(&q->lock);
    
    if (!is_full(q)) {
        q->buffer[q->head] = data;
        q->head = (q->head + 1) % QUEUE_SIZE;
        q->count++;
        status = 0; 
    } else {
        status = -1; 
    }
    
    mutex_unlock(&q->lock);
    return status;
}

int queue_receive(queue_t *q, uint32_t *data) {
    int status = -1;
    
    mutex_lock(&q->lock);
    
    if (!is_empty(q)) {
        *data = q->buffer[q->tail]; 
        q->tail = (q->tail + 1) % QUEUE_SIZE;
        q->count--;
        status = 0; 
    } else {
        status = -1; 
    }
    
    mutex_unlock(&q->lock);
    return status;
}