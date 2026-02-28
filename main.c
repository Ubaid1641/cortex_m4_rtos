#include "task1.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
   HARDWARE DEFINITIONS
   ============================================================================ */
#define RCC_BASE        0x40023800
#define GPIOA_BASE      0x40020000
#define GPIOD_BASE      0x40020C00
#define USART2_BASE     0x40004400

#define STK_CSR     (*(volatile uint32_t *)0xE000E010)
#define STK_RVR     (*(volatile uint32_t *)0xE000E014)
#define STK_CVR     (*(volatile uint32_t *)0xE000E018)
#define SCB_ICSR    (*(volatile uint32_t*)0xE000ED04)

#define RCC_AHB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x30))
#define RCC_APB1ENR     (*(volatile uint32_t*)(RCC_BASE + 0x40))
#define GPIOA_MODER     (*(volatile uint32_t*)(GPIOA_BASE + 0x00))
#define GPIOD_MODER     (*(volatile uint32_t*)(GPIOD_BASE + 0x00))
#define GPIOA_AFRL      (*(volatile uint32_t*)(GPIOA_BASE + 0x20))
#define GPIOD_ODR       (*(volatile uint32_t*)(GPIOD_BASE + 0x14))
#define USART2_SR       (*(volatile uint32_t*)(USART2_BASE + 0x00))
#define USART2_DR       (*(volatile uint32_t*)(USART2_BASE + 0x04))
#define USART2_BRR      (*(volatile uint32_t*)(USART2_BASE + 0x08))
#define USART2_CR1      (*(volatile uint32_t*)(USART2_BASE + 0x0C))

#define RCC_AHB1ENR_GPIOAEN  (1 << 0)
#define RCC_AHB1ENR_GPIODEN  (1 << 3)
#define RCC_APB1ENR_USART2EN (1 << 17)
#define STK_CSR_ENABLE    (1 << 0)
#define STK_CSR_TICKINT   (1 << 1)
#define STK_CSR_CLKSOURCE (1 << 2)

/* ============================================================================
   RTOS GLOBALS
   ============================================================================ */
volatile uint32_t system_ticks = 0;
TCB_t* volatile current_task = 0;

#define MAX_TASK 2
static TCB_t* task_list[MAX_TASK];
static uint8_t task_count = 0;

/* Stacks & TCBs */
uint32_t task1_stack[TASK_STACK_SIZE];
uint32_t task2_stack[TASK_STACK_SIZE];
TCB_t task1_tcb;
TCB_t task2_tcb;

/* ============================================================================
   RTOS CORE FUNCTIONS
   ============================================================================ */
void rtos_register_task(TCB_t* tcb) {
    if(task_count < MAX_TASK) {
        task_list[task_count++] = tcb;
    }
}

void rtos_sleep(uint32_t ms) {
    if (!current_task) return;

    __asm volatile ("cpsid i");
    current_task->state = BLOCKED;
    current_task->wake_time = system_ticks + ms;
    SCB_ICSR |= (1 << 28);
    __asm volatile ("cpsie i");
}

void PendSV_Handler_c(void) __attribute__((used));
void PendSV_Handler_c(void) {
    /* Save Current Context */
    __asm volatile (
        "MRS    r0, PSP           \n"
        "STR    r0, [%0]          \n"
        :
        : "r" (&current_task->sp)
        : "r0", "memory"
    );

    /* Find Next Ready Task */
    TCB_t* next_task = 0;
    uint8_t start_index = 0;

    for(int i = 0; i < task_count; i++) {
        if(task_list[i] == current_task) {
            start_index = i;
            break;
        }
    }

    for(int i = 1; i < task_count; i++) {
        uint8_t idx = (start_index + i) % task_count;
        TCB_t* candidate = task_list[idx];
        
        if(candidate->state == READY) {
            next_task = candidate;
            break;
        }
    }

    if(next_task == 0) {
        next_task = (TCB_t*)current_task;
    }

    current_task = next_task;

    /* Restore Next Context */
    __asm volatile (
        "LDR    r0, [%0]          \n"
        "MSR    PSP, r0           \n"
        "ISB                      \n"
        :
        : "r" (&current_task->sp)
        : "r0", "memory"
    );
}

__attribute__((naked)) void start_scheduler(TCB_t *first_task) {
    __asm volatile (
        "LDR    r0, [%0]          \n"
        "MSR    PSP, r0           \n"
        "MOV    r0, #2            \n"
        "MSR    CONTROL, r0       \n"
        "ISB                      \n"
        "LDR    r0, =0xFFFFFFFD   \n"
        "BX     r0                \n"
        :
        : "r" (first_task)
        : "r0"
    );
    __builtin_unreachable();
}

/* ============================================================================
   DRIVERS
   ============================================================================ */
static void system_init(void) {
    RCC_AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIODEN;
    RCC_APB1ENR |= RCC_APB1ENR_USART2EN;

    GPIOA_MODER &= ~(0x3 << 4); GPIOA_MODER |= (0x2 << 4);
    GPIOA_MODER &= ~(0x3 << 6); GPIOA_MODER |= (0x2 << 6);
    GPIOA_AFRL &= ~(0xF << 8);  GPIOA_AFRL |= (0x7 << 8);
    GPIOA_AFRL &= ~(0xF << 12); GPIOA_AFRL |= (0x7 << 12);

    GPIOD_MODER &= ~(0x3 << (2*12)); GPIOD_MODER |= (0x1 << (2*12));

    USART2_BRR = 0x8B; 
    USART2_CR1 |= (1 << 3) | (1 << 2) | (1 << 13);
}

void SysTick_Init(void) {
    STK_RVR = 15999; 
    STK_CVR = 0;
    STK_CSR = STK_CSR_CLKSOURCE | STK_CSR_TICKINT | STK_CSR_ENABLE;
}

void SysTick_Handler(void) {
    system_ticks++;

    for (int i = 0; i < task_count; i++) {
        TCB_t *tcb = task_list[i];
        if (tcb->state == BLOCKED && system_ticks >= tcb->wake_time) {
            tcb->state = READY;
        }
    }

    SCB_ICSR |= (1 << 28);
}

//=====================================================================================
void task1(void) {
    while (1) {
        USART2_DR = '1';
        while (!(USART2_SR & (1 << 7)));
        rtos_sleep(100);
    }
}

void task2(void) {
    while (1) {
        USART2_DR = '2';
        while (!(USART2_SR & (1 << 7)));
        rtos_sleep(100);
    }
}
/*============================================================================================*/
int main(void) {
    system_init();
    SysTick_Init();

    task_stack_init(&task1_tcb, task1, &task1_stack[TASK_STACK_SIZE]);
    task_stack_init(&task2_tcb, task2, &task2_stack[TASK_STACK_SIZE]);

    rtos_register_task(&task1_tcb);
    rtos_register_task(&task2_tcb);

    current_task = &task1_tcb;

    start_scheduler(&task1_tcb);

    while (1) { __asm__ volatile ("wfi"); }
}