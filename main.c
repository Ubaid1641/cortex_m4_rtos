
#include "task1.h"
#include <stdint.h>


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



volatile uint32_t tsk_1 = 0;
volatile uint32_t tsk_2 = 0;

volatile uint32_t task1_stack[TASK_STACK_SIZE];
volatile uint32_t task2_stack[TASK_STACK_SIZE];

volatile TCB_t task1_tcb;
volatile TCB_t task2_tcb;
void task1(void) {
    while (1) {  /* ← Tasks must loop forever */
        USART2_DR = '1';
        while (!(USART2_SR & (1 << 7)));
        for (volatile int i = 0; i < 50000; i++);
    }
}

void task2(void) {
    while (1) {
        USART2_DR = '2';
        while (!(USART2_SR & (1 << 7)));
        for (volatile int i = 0; i < 50000; i++);
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
// void HardFault_Handler(void) __attribute__((naked));
// void HardFault_Handler(void) {
//     
//     while(1) {
//         GPIOD_ODR ^= (1 << 12);  /* Toggle LED to show we hit HardFault */
//         for(volatile uint32_t i=0; i<1000000; i++);
//     }
// }
//void PendSV_Handler_c(void) __attribute__((used));
TCB_t* volatile current_task = 0;
void PendSV_Handler_c(void) __attribute__((used));
void PendSV_Handler_c(void){
    if(current_task == 0){
        return;
    }
    __asm__ volatile (
        "MRS r0,PSP \n"
        "STR r0,[%0] \n"
        :
        :"r"(&current_task->sp)
        :"r0","memory"
    );
    if(current_task == &task1_tcb){
        current_task = &task2_tcb;
    }
    else{
        current_task = &task1_tcb;
    }
    __asm__ volatile(
        "ldr r0,[%0] \n"
        "MSR PSP,r0 \n"
        "ISB \n"
        :
        :"r"(&current_task->sp)
        :"r0","memory"
    );
}
__attribute__((naked)) void start_scheduler(TCB_t *first_task) {
    __asm__ volatile (
        "MSR PSP, %0          \n"  // Load task's SP into PSP
        "MOV r0, #2           \n"  // CONTROL: use PSP, privileged
        "MSR CONTROL, r0      \n"
        "ISB                  \n"  // Sync pipeline
        "LDR r0, =0xFFFFFFFD  \n"  // ← EXC_RETURN: Thread Mode, PSP
        "BX r0                \n"  // ← Exception return to task
        :
        : "r" (first_task->sp)
        : "r0"
    );
    __builtin_unreachable();
}


void SysTick_Handler(void) {
     system_ticks++;
     if(system_ticks%50==0){
        SCB_ICSR |= (1<<28);
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
    current_task = &task1_tcb;
    start_scheduler(&task1_tcb);
      //__asm__("bkpt 0x01");
    while (1) {
        __asm__ volatile ("wfi");
    }
}