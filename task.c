#include "task1.h"

void task_stack_init(TCB_t *tcb, task_func_t func, uint32_t *stack_top){

    //aligning stack pointer to 8 bytes
    volatile uint32_t *stk = (uint32_t *)(((uint32_t)stack_top - 8) & ~0x7);
    
    //xPSR thumbset must be 0x01000000 otherwise it will give -->hardfault
    *--stk = 0x01000000;
    //PC must be in thumb mode,for this we have to set bit 0 to 1
    *--stk = (uint32_t)func | 0x1;
    //Link register LR = 0xFFFFFFFD --> this tells the program to return EXC_Return  
    //Its basically last 3 bits are 101 means to return to arm state,use PSP not MSP and return to thread mode
    *--stk = 0xFFFFFFFD;
    *--stk = 0; //r12
    *--stk = 0; //r3 
    *--stk = 0; //r2
    *--stk = 0; //r1
    *--stk = 0; //r0

    tcb->sp = stk;
} 