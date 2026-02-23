#ifndef TASK_H
#define TASK_H

#include <stdint.h>


#define TASK_STACK_SIZE 256

typedef struct{
    uint32_t* sp;

}TCB_t;

typedef void (*task_func_t)(void);

void task_stack_init(TCB_t *tcb, task_func_t func, uint32_t *stack_top);
#endif