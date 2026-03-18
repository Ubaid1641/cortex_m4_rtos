#include "task1.h"
#include "mutex.h"
#include <stdint.h>
#include "queue.h"
#define MAX_TASKS 4
static TCB_t* task_list[MAX_TASKS];
uint8_t count = 0;
volatile uint32_t system_ticks = 0;

#define RCC_BASE        0x40023800
#define GPIOA_BASE      0x40020000
#define GPIOD_BASE      0x40020C00
#define USART2_BASE     0x40004400

#define STK_CSR     (*(volatile uint32_t *)0xE000E010)
#define STK_RVR     (*(volatile uint32_t *)0xE000E014)
#define STK_CVR     (*(volatile uint32_t *)0xE000E018)
#define SCB_ICSR    (*(volatile uint32_t*)0xE000ED04)

#define STK_CSR_ENABLE    (1 << 0)
#define STK_CSR_TICKINT   (1 << 1)
#define STK_CSR_CLKSOURCE (1 << 2)

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

void print_string(const char* str);
void print_num(uint32_t num);

TCB_t* volatile current_task = 0;

uint32_t task1_stack[TASK_STACK_SIZE];
uint32_t task2_stack[TASK_STACK_SIZE];
uint32_t task3_stack[TASK_STACK_SIZE];
uint32_t idl_tsk_stack[TASK_STACK_SIZE];
uint32_t logger_task_stack[TASK_STACK_SIZE];

TCB_t task1_tcb;
TCB_t task2_tcb;
TCB_t task3_tcb;
TCB_t idl_tsk_tcb;
TCB_t logger_task_tcb;
mutex_t uart_mutex;
queue_t queue_local;
void rtos_sleep(uint32_t ms){
    if(current_task){
        current_task->state = BLOCKED;
        current_task->wake_time = system_ticks + ms;
    }
    SCB_ICSR |= (1 << 28);
}

void rtos_register_task(TCB_t* tcb){
    if(count < MAX_TASKS){
        task_list[count++] = tcb;
    }
}

void task1(void) {
    while (1) {
        
        queue_send(&queue_local, 123);
        
      
        
        rtos_sleep(1000); 
    }
}

void task2(void) {
    while (1) {
        
        queue_send(&queue_local, 555);
        
        
        
        rtos_sleep(5000); 
    }
}
void logger_task(void) {
    uint32_t data;
    int status;
    
    while (1) {
        status = queue_receive(&queue_local, &data);
        
        if (status == 0) {
            
            print_string("\r\n[LOG] Data: ");
            print_num(data);
        } else {
           
            rtos_sleep(10); 
        }
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

void SysTick_Init(void) {
    uint32_t reload_val = 15999;
    STK_RVR = reload_val;
    STK_CVR = 0;
    STK_CSR = STK_CSR_CLKSOURCE | STK_CSR_TICKINT | STK_CSR_ENABLE;
}
void PrintHardFaultInfo(uint32_t *stack_frame) {
    uint32_t pc = stack_frame[6];  /* PC is at offset 24 (6 words) */
    print_string("\r\n[HARDFAULT] PC: ");
    print_num(pc);
    print_string("\r\n");
}
void scheduler_yield(void) {
    SCB_ICSR |= (1 << 28);
}

void PendSV_Handler_c(void) __attribute__((used));
void PendSV_Handler_c(void) {
    if (current_task == 0) return;

    __asm__ volatile (
        "MRS r0, PSP\n"
        "STR r0, [%0]\n"
        : : "r" (&current_task->sp) : "r0", "memory"
    );

    TCB_t *next_task = 0;

    for (int i = 0; i < count; i++) {
        TCB_t *candidate = task_list[i];
        if (candidate == current_task) continue;
        if (candidate->state != READY) continue;
        next_task = candidate;
        break;
    }

    if (next_task == 0) {
        for (int i = 0; i < count; i++) {
            if (task_list[i]->priority == 0 && task_list[i]->state == READY) {
                next_task = task_list[i];
                break;
            }
        }
    }

    if (next_task == 0) {
        next_task = current_task;
    }

    current_task = next_task;

    __asm__ volatile (
        "LDR r0, [%0]\n"
        "MSR PSP, r0\n"
        "ISB\n"
        : : "r" (&current_task->sp) : "r0", "memory"
    );
}

__attribute__((naked)) void start_scheduler(TCB_t *first_task) {
    __asm__ volatile (
        "MSR PSP, %0\n"
        "MOV r0, #2\n"
        "MSR CONTROL, r0\n"
        "ISB\n"
        "LDR r0, =0xFFFFFFFD\n"
        "BX r0\n"
        :
        : "r" (first_task->sp)
        : "r0"
    );
    __builtin_unreachable();
}
void kernel_panic(const char* r){
       __asm volatile("cpsid i");
       print_string("\r\n[PANIC]");
       print_string(r);
       print_string("\r\n");
       while(1);
}
void check_stack_canary(TCB_t* tcb){
     if(tcb->stack_start[0]!=0xDEADBEEF){
          kernel_panic("stack overflowed");
     }
}

void SysTick_Handler_c(void) {
    system_ticks++;
    uint8_t task_woken = 0;

    for (int i = 0; i < count; i++) {
        TCB_t *tcb = task_list[i];
        if (tcb->state == BLOCKED && system_ticks >= tcb->wake_time) {
            tcb->state = READY;
            task_woken = 1;
        }
    }
    for(int i=0;i<count;i++){
          TCB_t* t = task_list[i];
          check_stack_canary(t);
    }

    if (task_woken) {
        SCB_ICSR |= (1 << 28);
    } else if (system_ticks % 10 == 0) {
        SCB_ICSR |= (1 << 28);
    }
    
}

int main(void) {
    system_init();
    SysTick_Init();
    queue_init(&queue_local);
    //mutex_init(&uart_mutex, 2);

    task_stack_init(&task1_tcb, task1, &task1_stack[TASK_STACK_SIZE], 3);
    task_stack_init(&task2_tcb, task2, &task2_stack[TASK_STACK_SIZE], 2);
    task_stack_init(&idl_tsk_tcb, idle_task, &idl_tsk_stack[TASK_STACK_SIZE], 0);
    task_stack_init(&logger_task_tcb, logger_task, &logger_task_stack[TASK_STACK_SIZE], 1);
    rtos_register_task(&task1_tcb);
    rtos_register_task(&task2_tcb);
    rtos_register_task(&idl_tsk_tcb);
    rtos_register_task(&logger_task_tcb);

    current_task = &task1_tcb;
    start_scheduler(&task1_tcb);

    while (1) {
        __asm__ volatile ("wfi");
    }
}

void print_string(const char* str) {
    while (*str) {
        USART2_DR = *str++;
        while (!(USART2_SR & (1 << 7)));
    }
}

void print_num(uint32_t num) {
    char buf[16];
    int i = 0;
    if (num == 0) {
        print_string("0");
        return;
    }
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    while (i > 0) {
        USART2_DR = buf[--i];
        while (!(USART2_SR & (1 << 7)));
    }
}
