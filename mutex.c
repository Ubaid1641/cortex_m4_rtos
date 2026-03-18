#include "mutex.h"
#include "task1.h"

static void critical_enter(void) {
    __asm volatile ("cpsid i" : : : "memory");
    __asm volatile ("dsb" : : : "memory");
    __asm volatile ("isb" : : : "memory");
}

static void critical_exit(void) {
    __asm volatile ("isb" : : : "memory");
    __asm volatile ("cpsie i" : : : "memory");
}

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
        m->state = LOCKED;
        m->owner = current_task;
        m->owner->base_priority = m->owner->priority;
        critical_exit();
        return;
    }
    mutex_add_waiter(m, current_task);
    if (current_task->priority > m->owner->priority) {
        m->owner->priority = current_task->priority;
    }
    current_task->state = BLOCKED;
    current_task->waiting_on = m;
    critical_exit();
    scheduler_yield();
}

void mutex_unlock(mutex_t* m) {
    critical_enter();
    if (m->owner != current_task) {
        critical_exit();
        return;
    }
    if (current_task->priority != current_task->base_priority) {
        current_task->priority = current_task->base_priority;
    }
    TCB_t* next_owner = mutex_remove_waiter(m);
    if (next_owner) {
        m->owner = next_owner;
        next_owner->state = READY;
        next_owner->waiting_on = 0;
        next_owner->base_priority = next_owner->priority;
        critical_exit();
        if (next_owner->priority > current_task->priority) {
            scheduler_yield();
        }
    } else {
        m->state = UNLOCKED;
        m->owner = 0;
        critical_exit();
    }
}