#ifndef TASK_H
#define TASK_H

#include <stdint.h>


#define TASK_STACK_SIZE 256
typedef enum{
    READY,
    BLOCKED
}task_state;
typedef struct{
    uint32_t* sp;
    task_state state;
    uint32_t wake_time;
}TCB_t;

typedef void (*task_func_t)(void);

void task_stack_init(TCB_t *tcb, task_func_t func, uint32_t *stack_top);
#endif