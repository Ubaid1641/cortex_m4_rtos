
#include "task1.h"
#include <stdint.h>

#define MAX_TASKS 3
static TCB_t* task_list[MAX_TASKS];
uint8_t count = 0;
volatile uint32_t system_ticks = 0;

#define RCC_BASE        0x40023800
#define GPIOA_BASE      0x40020000
#define GPIOD_BASE      0x40020C00
#define USART2_BASE     0x40004400

/*Systick Reisters*/
#define STK_CSR     (*(volatile uint32_t *)0xE000E010)  /* Control & Status */
#define STK_RVR     (*(volatile uint32_t *)0xE000E014)  /* Reload Value */
#define STK_CVR     (*(volatile uint32_t *)0xE000E018)  /* Current Value */
#define SCB_ICSR (*(volatile uint32_t*)0xE000ED04)
/* SysTick Control Bits */
#define STK_CSR_ENABLE    (1 << 0)  /* Enable counter */
#define STK_CSR_TICKINT   (1 << 1)  /* Enable exception request */
#define STK_CSR_CLKSOURCE (1 << 2)  /* 1 = Processor Clock, 0 = External */
/* RCC Registers */
#define RCC_AHB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x30))
#define RCC_APB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x40))

/* GPIOA Registers */
#define GPIOA_MODER     (*(volatile uint32_t*)(GPIOA_BASE + 0x00))
#define GPIOD_MODER     (*(volatile uint32_t*)(GPIOD_BASE + 0x00))
#define GPIOA_AFRL      (*(volatile uint32_t*)(GPIOA_BASE + 0x20))

#define GPIOD_ODR  (*(volatile uint32_t*)(GPIOD_BASE + 0x14))

/* USART2 Registers */
#define USART2_SR       (*(volatile uint32_t*)(USART2_BASE + 0x00))
#define USART2_DR       (*(volatile uint32_t*)(USART2_BASE + 0x04))
#define USART2_BRR      (*(volatile uint32_t*)(USART2_BASE + 0x08))
#define USART2_CR1      (*(volatile uint32_t*)(USART2_BASE + 0x0C)) /* Offset 0x0C is CR1 on some, check ref */

#define RCC_AHB1ENR_GPIOAEN  (1 << 0)
#define RCC_AHB1ENR_GPIODEN  (1 << 3)
#define RCC_APB1ENR_USART2EN (1 << 17)


TCB_t* volatile current_task = 0;
volatile uint32_t tsk_1 = 0;
volatile uint32_t tsk_2 = 0;
volatile uint32_t idl_tsk = 0;
 uint32_t task1_stack[TASK_STACK_SIZE];
 uint32_t task2_stack[TASK_STACK_SIZE];
uint32_t idl_tsk_stack[TASK_STACK_SIZE];
 TCB_t task1_tcb;
 TCB_t task2_tcb;
 TCB_t idl_tsk_tcb;
void rtos_sleep(uint32_t ms){
    if(current_task){
        current_task->state = BLOCKED;
        current_task->wake_time = system_ticks + ms;
    }
    *((volatile uint32_t*)0xE000ED04) |= (1 << 28);
}
void rtos_register_task(TCB_t* tcb){
    if(count < MAX_TASKS){
        task_list[count++] = tcb;
    }
}

void task1(void) {
    while (1) {  /* ← Tasks must loop forever */
        USART2_DR = '1';
        while (!(USART2_SR & (1 << 7)));
        rtos_sleep(100);
    }
}

void task2(void) {
    while (1) {
        USART2_DR = '2';
        while (!(USART2_SR & (1 << 7)));
        rtos_sleep(50);
    }
}
void idle_task(void){
    while(1){
        __asm__ volatile ("wfi");
    }
}
static void system_init(void) {
   
    RCC_AHB1ENR |= RCC_AHB1ENR_GPIOAEN;   
    RCC_AHB1ENR |= RCC_AHB1ENR_GPIODEN;   
    RCC_APB1ENR |= RCC_APB1ENR_USART2EN;  


    GPIOA_MODER &= ~(0x3 << 4);
    GPIOA_MODER |= (0x2 << 4);

    GPIOD_MODER &= ~(0x3 << (2*12));
    GPIOD_MODER |= (0x1 << (2*12));

    GPIOA_AFRL &= ~(0xF << 8);
    GPIOA_AFRL |= (0x7 << 8);


    USART2_BRR = 0x8B; 
    
    
    USART2_CR1 |= (1 << 3);  
    USART2_CR1 |= (1 << 2);  
    USART2_CR1 |= (1 << 13); 
}


static void uart_putc(char c) {
    /* Wait until Transmit Data Register Empty (TXE = bit 7) */
    while (!(USART2_SR & (1 << 7))); 
    USART2_DR = c;
}

static void print(char *s) {
    while (*s) uart_putc(*s++);
}
void SysTick_Init(void) {

    uint32_t reload_val = 15999; 

    STK_RVR = reload_val;

    STK_CVR = 0;

    STK_CSR = STK_CSR_CLKSOURCE | STK_CSR_TICKINT | STK_CSR_ENABLE;
}


void PendSV_Handler_c(void) __attribute__((used));
void PendSV_Handler_c(void) {
    if (current_task == 0) return;
    
    /* 1. Save current task's PSP */
    __asm__ volatile (
        "MRS r0, PSP          \n"
        "STR r0, [%0]         \n"
        : : "r" (&current_task->sp) : "r0", "memory"
    );
    
    /* 2. Find next READY task (simple round-robin for now) */
    TCB_t *next_task = 0;
    
    /* Try each registered task */
    for (int i = 0; i < count; i++) {
        TCB_t *candidate = task_list[i];
        
        /* Skip if not current task and not ready */
        if (candidate == current_task) continue;
        if (candidate->state != READY) continue;
        
        /* Found a ready task */
        next_task = candidate;
        break;
    }
    
    /* If no ready task found, stay on current (it might be waking now) */
    if (next_task == 0) {
        next_task = current_task;
    }
    
    /* Update current_task pointer */
    current_task = next_task;
    
    /* 3. Load next task's PSP */
    __asm__ volatile (
        "LDR r0, [%0]         \n"
        "MSR PSP, r0          \n"
        "ISB                  \n"
        : : "r" (&current_task->sp) : "r0", "memory"
    );
    
    /* 4. Return: hardware restores registers from new PSP */
}
__attribute__((naked)) void start_scheduler(TCB_t *first_task) {
    __asm__ volatile (
        "MSR PSP, %0          \n"  
        "MOV r0, #2           \n"  
        "MSR CONTROL, r0      \n"
        "ISB                  \n"  
        "LDR r0, =0xFFFFFFFD  \n"
        "BX r0                \n"  
        :
        : "r" (first_task->sp)
        : "r0"
    );
    __builtin_unreachable();
}


void SysTick_Handler(void) {
     system_ticks++;
         for (int i = 0; i < count; i++) {
        TCB_t *tcb = task_list[i];
        if (tcb->state == BLOCKED && system_ticks >= tcb->wake_time) {
            tcb->state = READY;
        }
    }
    
    /* Existing: Trigger context switch every N ticks */
    if (system_ticks % 10 == 0) {
        *((volatile uint32_t*)0xE000ED04) |= (1 << 28);  /* PendSV */
    }

}
//volatile uint32_t debug_heartbeat = 0;
/* --- Entry --- */
int main(void) {
    system_init();
    SysTick_Init();
    //Systick_Handler();
    //print("Day 1: RTOS journey begins\n");
    
    task_stack_init(&task1_tcb, task1, &task1_stack[TASK_STACK_SIZE]);
    task_stack_init(&task2_tcb, task2, &task2_stack[TASK_STACK_SIZE]);
    task_stack_init(&idl_tsk_tcb,idle_task,&idl_tsk_stack[TASK_STACK_SIZE]);
    rtos_register_task(&task1_tcb);
    rtos_register_task(&task2_tcb);
    rtos_register_task(&idl_tsk_tcb);
    current_task = &task1_tcb;
    start_scheduler(&task1_tcb);
      //__asm__("bkpt 0x01");
    while (1) {
        __asm__ volatile ("wfi");
    }
}