#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>
#include "task1.h"
typedef enum{
    LOCKED=1,
    UNLOCKED=0
}mutex_state_t;

typedef struct Mutex{
   mutex_state_t state;
   struct TCB* owner;
   struct TCB* wait_list_head;
   uint32_t id;
}mutex_t;

void mutex_init(mutex_t* m,uint32_t id);
void mutex_lock(mutex_t* m);
void mutex_unlock(mutex_t* m);
#endif