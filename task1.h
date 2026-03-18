#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "mutex.h"

#define TASK_STACK_SIZE 256

typedef enum{
    READY,
    BLOCKED,
    RUNNING
} task_state;

typedef struct TCB{
    uint32_t* sp;
    uint32_t* stack_start;
    task_state state;
    uint32_t wake_time;
    struct TCB* next;
    struct Mutex* waiting_on;
    uint32_t priority;
    uint32_t base_priority;
} TCB_t;

typedef void (*task_func_t)(void);

extern TCB_t* volatile current_task;
extern volatile uint32_t system_ticks;

void task_stack_init(TCB_t *tcb, task_func_t func, uint32_t *stack_top, uint32_t priority);
void rtos_register_task(TCB_t* tcb);
void rtos_sleep(uint32_t ms);
void scheduler_yield(void);

#endif