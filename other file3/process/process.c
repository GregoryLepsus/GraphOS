// ============================================================================
// Process Management
// Управление процессами, PCB, scheduler
// ============================================================================

#include "../include/process.h"
#include "../include/heap.h"
#include "../include/pmm.h"
#include "../include/paging.h"
#include "../include/timer.h"

// Внешние функции
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void print_dec(uint32_t value);
extern void print_hex(uint32_t value);

// Глобальные переменные
process_t* current_process = NULL;
uint32_t next_pid = 1;

// Очереди процессов
static process_t* ready_queue_head = NULL;
static process_t* ready_queue_tail = NULL;
static process_t* blocked_queue_head = NULL;

// Idle process
static process_t* idle_process = NULL;

// Статистика
static uint32_t total_processes = 0;
static uint32_t context_switches = 0;

// ============================================================================
// Функция: process_queue_add
// Добавляет процесс в очередь
// ============================================================================
static void process_queue_add(process_t** head, process_t** tail, process_t* proc) {
    proc->next = NULL;
    proc->prev = *tail;

    if (*tail) {
        (*tail)->next = proc;
    }
    *tail = proc;

    if (!*head) {
        *head = proc;
    }
}

// ============================================================================
// Функция: process_queue_remove
// Удаляет процесс из очереди
// ============================================================================
static void process_queue_remove(process_t** head, process_t** tail, process_t* proc) {
    if (proc->prev) {
        proc->prev->next = proc->next;
    } else {
        *head = proc->next;
    }

    if (proc->next) {
        proc->next->prev = proc->prev;
    } else {
        *tail = proc->prev;
    }

    proc->next = NULL;
    proc->prev = NULL;
}

// ============================================================================
// Функция: process_queue_pop
// Извлекает первый процесс из очереди
// ============================================================================
static process_t* process_queue_pop(process_t** head, process_t** tail) {
    if (!*head) {
        return NULL;
    }

    process_t* proc = *head;
    process_queue_remove(head, tail, proc);
    return proc;
}

// ============================================================================
// Функция: idle_task
// Idle процесс (выполняется, когда нет других процессов)
// ============================================================================
static void idle_task(void) {
    while (1) {
        __asm__ volatile("hlt");
    }
}

// ============================================================================
// Функция: process_init
// Инициализирует систему управления процессами
// ============================================================================
void process_init(void) {
    terminal_write_line("[PROCESS] Initializing process management...");

    // Создать idle процесс
    idle_process = (process_t*)kmalloc(sizeof(process_t));
    if (!idle_process) {
        terminal_write_line("[PROCESS] ERROR: Cannot allocate idle process!");
        return;
    }

    idle_process->pid = 0;
    for (int i = 0; i < 32; i++) idle_process->name[i] = 0;
    idle_process->name[0] = 'i'; idle_process->name[1] = 'd';
    idle_process->name[2] = 'l'; idle_process->name[3] = 'e';
    idle_process->state = PROCESS_READY;
    idle_process->priority = 7;  // Lowest priority

    // Kernel space, no user stack
    idle_process->kernel_stack = 0;
    idle_process->user_stack = 0;
    idle_process->page_directory = kernel_directory;

    idle_process->sleep_until = 0;
    idle_process->next = NULL;
    idle_process->prev = NULL;

    // Инициализация контекста idle процесса
    idle_process->context.eip = (uint32_t)idle_task;
    idle_process->context.esp = 0;  // Will be set properly
    idle_process->context.ebp = 0;
    idle_process->context.eflags = 0x202;  // IF flag set

    current_process = idle_process;
    total_processes = 1;

    terminal_write_line("[OK] Process management initialized");
}

// ============================================================================
// Функция: process_create
// Создает новый процесс
// ============================================================================
process_t* process_create(const char* name, void (*entry_point)(void), uint32_t priority) {
    // Выделить PCB
    process_t* proc = (process_t*)kmalloc(sizeof(process_t));
    if (!proc) {
        terminal_write_line("[PROCESS] ERROR: Cannot allocate PCB!");
        return NULL;
    }

    // Инициализировать PCB
    proc->pid = next_pid++;

    // Скопировать имя
    int i;
    for (i = 0; i < 31 && name[i]; i++) {
        proc->name[i] = name[i];
    }
    proc->name[i] = '\0';

    proc->state = PROCESS_READY;
    proc->priority = priority;

    // Выделить kernel stack (4KB)
    proc->kernel_stack = (uint32_t)kmalloc(4096) + 4096;  // Stack grows down
    if (proc->kernel_stack == 4096) {
        kfree(proc);
        terminal_write_line("[PROCESS] ERROR: Cannot allocate kernel stack!");
        return NULL;
    }

    // Clone page directory from kernel
    proc->page_directory = paging_clone_directory(kernel_directory);
    if (!proc->page_directory) {
        kfree((void*)(proc->kernel_stack - 4096));
        kfree(proc);
        terminal_write_line("[PROCESS] ERROR: Cannot clone page directory!");
        return NULL;
    }

    // Инициализировать контекст
    proc->context.eip = (uint32_t)entry_point;
    proc->context.esp = proc->kernel_stack;
    proc->context.ebp = proc->kernel_stack;
    proc->context.eflags = 0x202;  // IF flag set
    proc->context.cr3 = (uint32_t)proc->page_directory;

    // Обнулить регистры
    proc->context.eax = 0;
    proc->context.ebx = 0;
    proc->context.ecx = 0;
    proc->context.edx = 0;
    proc->context.esi = 0;
    proc->context.edi = 0;

    proc->user_stack = 0;  // No user stack yet
    proc->sleep_until = 0;
    proc->next = NULL;
    proc->prev = NULL;

    // Добавить в ready queue
    scheduler_add_process(proc);
    total_processes++;

    return proc;
}

// ============================================================================
// Функция: process_exit
// Завершает текущий процесс
// ============================================================================
void process_exit(int exit_code) {
    (void)exit_code;  // Unused for now

    if (!current_process || current_process == idle_process) {
        return;  // Cannot exit idle process
    }

    terminal_write("[PROCESS] Process ");
    print_dec(current_process->pid);
    terminal_write(" (");
    terminal_write(current_process->name);
    terminal_write_line(") exiting");

    current_process->state = PROCESS_DEAD;
    total_processes--;

    // TODO: Free resources (stack, page directory, etc)

    // Switch to another process
    schedule();
}

// ============================================================================
// Функция: process_get_current
// Возвращает текущий процесс
// ============================================================================
process_t* process_get_current(void) {
    return current_process;
}

// ============================================================================
// Функция: process_get_by_pid
// Находит процесс по PID
// ============================================================================
process_t* process_get_by_pid(uint32_t pid) {
    // Search ready queue
    process_t* proc = ready_queue_head;
    while (proc) {
        if (proc->pid == pid) {
            return proc;
        }
        proc = proc->next;
    }

    // Search blocked queue
    proc = blocked_queue_head;
    while (proc) {
        if (proc->pid == pid) {
            return proc;
        }
        proc = proc->next;
    }

    // Check current process
    if (current_process && current_process->pid == pid) {
        return current_process;
    }

    return NULL;
}

// ============================================================================
// Функция: process_sleep
// Переводит текущий процесс в сон
// ============================================================================
void process_sleep(uint32_t ticks) {
    if (!current_process || current_process == idle_process) {
        return;
    }

    current_process->sleep_until = timer_ticks() + ticks;
    current_process->state = PROCESS_BLOCKED;

    // Add to blocked queue
    process_queue_add(&blocked_queue_head, NULL, current_process);

    // Switch to another process
    schedule();
}

// ============================================================================
// Функция: scheduler_init
// Инициализирует scheduler
// ============================================================================
void scheduler_init(void) {
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    blocked_queue_head = NULL;
    context_switches = 0;
}

// ============================================================================
// Функция: scheduler_add_process
// Добавляет процесс в ready queue
// ============================================================================
void scheduler_add_process(process_t* proc) {
    proc->state = PROCESS_READY;
    process_queue_add(&ready_queue_head, &ready_queue_tail, proc);
}

// ============================================================================
// Функция: scheduler_remove_process
// Удаляет процесс из ready queue
// ============================================================================
void scheduler_remove_process(process_t* proc) {
    process_queue_remove(&ready_queue_head, &ready_queue_tail, proc);
}

// ============================================================================
// Функция: scheduler_wake_sleeping
// Будит процессы, которые проспали достаточно
// ============================================================================
static void scheduler_wake_sleeping(void) {
    uint32_t now = timer_ticks();
    process_t* proc = blocked_queue_head;

    while (proc) {
        process_t* next = proc->next;

        if (proc->sleep_until > 0 && now >= proc->sleep_until) {
            // Wake up this process
            proc->sleep_until = 0;
            process_queue_remove(&blocked_queue_head, NULL, proc);
            scheduler_add_process(proc);
        }

        proc = next;
    }
}

// ============================================================================
// Функция: schedule
// Выбирает следующий процесс для выполнения (Round Robin)
// ============================================================================
void schedule(void) {
    // Wake sleeping processes
    scheduler_wake_sleeping();

    // Get next process from ready queue
    process_t* next = process_queue_pop(&ready_queue_head, &ready_queue_tail);

    // If no process available, use idle
    if (!next) {
        next = idle_process;
    }

    // If same process, just return
    if (next == current_process) {
        if (current_process->state == PROCESS_READY) {
            scheduler_add_process(current_process);
        }
        return;
    }

    // Save old process
    process_t* old = current_process;

    // If old process is still ready, add back to queue
    if (old && old->state == PROCESS_RUNNING) {
        old->state = PROCESS_READY;
        if (old != idle_process) {
            scheduler_add_process(old);
        }
    }

    // Switch to new process
    current_process = next;
    next->state = PROCESS_RUNNING;
    context_switches++;

    // Perform context switch
    if (old) {
        switch_context(&old->context, &next->context);
    }
}

// ============================================================================
// Функция: process_list
// Выводит список процессов
// ============================================================================
void process_list(void) {
    terminal_write_line("Process List:");
    terminal_write_line("  PID  State    Priority  Name");

    // Current process
    if (current_process) {
        terminal_write("  ");
        print_dec(current_process->pid);
        terminal_write("  RUNNING  ");
        print_dec(current_process->priority);
        terminal_write("         ");
        terminal_write_line(current_process->name);
    }

    // Ready queue
    process_t* proc = ready_queue_head;
    while (proc) {
        terminal_write("  ");
        print_dec(proc->pid);
        terminal_write("  READY    ");
        print_dec(proc->priority);
        terminal_write("         ");
        terminal_write_line(proc->name);
        proc = proc->next;
    }

    // Blocked queue
    proc = blocked_queue_head;
    while (proc) {
        terminal_write("  ");
        print_dec(proc->pid);
        terminal_write("  BLOCKED  ");
        print_dec(proc->priority);
        terminal_write("         ");
        terminal_write_line(proc->name);
        proc = proc->next;
    }

    terminal_write("Total processes: ");
    print_dec(total_processes);
    terminal_write(", Context switches: ");
    print_dec(context_switches);
    terminal_write_line("");
}
