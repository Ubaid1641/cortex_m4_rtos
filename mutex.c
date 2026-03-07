#include "mutex.h"
#include "task1.h"

// Simple critical section
static void critical_enter(void) {
    __asm volatile ("cpsid i" : : : "memory");  // Disable interrupts
    __asm volatile ("dsb" : : : "memory");      // Data sync barrier
    __asm volatile ("isb" : : : "memory");      // Instruction sync barrier
}

static void critical_exit(void) {
    __asm volatile ("isb" : : : "memory");      // Instruction sync barrier
    __asm volatile ("cpsie i" : : : "memory");  // Enable interrupts
}

// Add task to end of wait list
static void mutex_add_waiter(mutex_t* m, TCB_t* task) {
    task->next = 0;
    
    if (m->wait_list_head == 0) {
        m->wait_list_head = task;
    } else {
        TCB_t* tail = m->wait_list_head;
        while (tail->next != 0) {
            tail = tail->next;
        }
        tail->next = task;
    }
}

// Remove first task from wait list
static TCB_t* mutex_remove_waiter(mutex_t* m) {
    if (m->wait_list_head == 0) {
        return 0;
    }
    
    TCB_t* task = m->wait_list_head;
    m->wait_list_head = task->next;
    task->next = 0;
    return task;
}

void mutex_init(mutex_t* m, uint32_t id) {
    m->state = UNLOCKED;
    m->owner = 0;
    m->wait_list_head = 0;
    m->id = id;
}

void mutex_lock(mutex_t* m) {
    critical_enter();
    
    if (m->state == UNLOCKED) {
        // Mutex is free - take it
        m->state = LOCKED;
        m->owner = current_task;
        m->owner->base_priority = m->owner->priority; // Store base priority
        critical_exit();
        return;
    }
    
    // Mutex is locked - must wait
    // Add to wait list
    mutex_add_waiter(m, current_task);
    
    // Simple priority inheritance: boost owner if needed
    if (current_task->priority > m->owner->priority) {
        m->owner->priority = current_task->priority;
    }
    
    // Block this task
    current_task->state = BLOCKED;
    current_task->waiting_on = m;
    
    critical_exit();
    
    // Give up CPU - will return here when mutex is available
    scheduler_yield();
}

void mutex_unlock(mutex_t* m) {
    critical_enter();
    
    // Safety check - only owner can unlock
    if (m->owner != current_task) {
        critical_exit();
        return;
    }
    
    // Restore owner's priority if it was boosted
    if (current_task->priority != current_task->base_priority) {
        current_task->priority = current_task->base_priority;
    }
    
    // Check if anyone is waiting
    TCB_t* next_owner = mutex_remove_waiter(m);
    
    if (next_owner) {
        // Pass mutex to next waiting task
        m->owner = next_owner;
        next_owner->state = READY;
        next_owner->waiting_on = 0;
        next_owner->base_priority = next_owner->priority; // Store its base priority
        
        critical_exit();
        
        // If next owner has higher priority, yield now
        if (next_owner->priority > current_task->priority) {
            scheduler_yield();
        }
    } else {
        // No waiters - just unlock
        m->state = UNLOCKED;
        m->owner = 0;
        critical_exit();
    }
}