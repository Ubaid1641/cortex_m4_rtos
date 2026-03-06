#include "task1.h"

void task_stack_init(TCB_t *tcb, task_func_t func, uint32_t *stack_top){

    
    volatile uint32_t *stk = (uint32_t *)(((uint32_t)stack_top - 8) & ~0x7);
    
    stk-=8;
    *--stk = 0x01000000;
    
    *--stk = (uint32_t)func | 0x1;
    
    *--stk = 0xFFFFFFFD;
    *--stk = 0; //r12
    *--stk = 0; //r3 
    *--stk = 0; //r2
    *--stk = 0; //r1
    *--stk = 0; //r0

    tcb->sp = stk;
    tcb->state = READY;
    tcb->wake_time = 0;
} 