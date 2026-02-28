#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stdbool.h>

#define TASK_STACK_SIZE 256

typedef enum {
    BLOCKED,
    READY
} task_state;

typedef struct {
    uint32_t* sp;
    task_state state;
    uint8_t priority;
    uint32_t wake_time;
} TCB_t;

typedef void (*task_func_t)(void);

/* Function Prototypes */
void task_stack_init(TCB_t *tcb, task_func_t func, uint32_t *stack_top);
void task1(void);
void task2(void);
void rtos_sleep(uint32_t ms);

#endif