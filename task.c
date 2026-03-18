#include "task1.h"

void task_stack_init(TCB_t *tcb, task_func_t func, uint32_t *stack_top, uint32_t priority){
    volatile uint32_t *stk = (uint32_t *)(((uint32_t)stack_top - 8) & ~0x7);
    stk -= 8;
    *--stk = 0x01000000;
    *--stk = (uint32_t)func | 0x1;
    *--stk = 0xFFFFFFFD;
    *--stk = 0;
    *--stk = 0;
    *--stk = 0;
    *--stk = 0;
    *--stk = 0;
    
    uint32_t* stack_bottom = stack_top - TASK_STACK_SIZE;
    stack_bottom[0] = 0xDEADBEEF;
    tcb->stack_start = stack_bottom;
    tcb->sp = (uint32_t*)stk;
    tcb->state = READY;
    tcb->wake_time = 0;
    tcb->next = 0;
    tcb->waiting_on = 0;
    tcb->priority = priority;
    tcb->base_priority = priority;
}
