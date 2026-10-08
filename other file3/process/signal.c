// ============================================================================
// Signal System
// POSIX-like signal handling
// ============================================================================

#include "../include/signal.h"
#include "../include/process.h"

extern process_t* process_get(uint32_t pid);

// ============================================================================
// Signal Initialization
// ============================================================================

void signal_init(process_t* proc) {
    if (!proc) return;

    // Initialize signal info (would be in process structure)
    // proc->signals.pending = 0;
    // proc->signals.blocked = 0;

    // Set default handlers
    for (int i = 0; i < MAX_SIGNALS; i++) {
        // proc->signals.handlers[i] = NULL;
    }
}

// ============================================================================
// Signal Delivery
// ============================================================================

int signal_send(uint32_t pid, int signum) {
    if (signum < 1 || signum >= MAX_SIGNALS) {
        return -1;
    }

    process_t* proc = process_get(pid);
    if (!proc) {
        return -1;
    }

    // Mark signal as pending
    // proc->signals.pending |= (1 << signum);

    // Handle immediately if not blocked
    // if (!(proc->signals.blocked & (1 << signum))) {
    //     signal_handle_pending(proc);
    // }

    return 0;
}

// ============================================================================
// Signal Handlers
// ============================================================================

int signal_set_handler(process_t* proc, int signum, signal_handler_t handler) {
    if (!proc || signum < 1 || signum >= MAX_SIGNALS) {
        return -1;
    }

    // Cannot catch SIGKILL or SIGSTOP
    if (signum == SIGKILL || signum == SIGSTOP) {
        return -1;
    }

    // proc->signals.handlers[signum] = handler;
    return 0;
}

void signal_handle_pending(process_t* proc) {
    if (!proc) return;

    // Check for pending signals
    // for (int i = 1; i < MAX_SIGNALS; i++) {
    //     if ((proc->signals.pending & (1 << i)) &&
    //         !(proc->signals.blocked & (1 << i))) {
    //
    //         // Clear pending bit
    //         proc->signals.pending &= ~(1 << i);
    //
    //         // Call handler if set
    //         if (proc->signals.handlers[i]) {
    //             proc->signals.handlers[i](i);
    //         } else {
    //             // Default action
    //             if (i == SIGKILL || i == SIGTERM) {
    //                 // Terminate process
    //             }
    //         }
    //     }
    // }
}

// ============================================================================
// Signal Masking
// ============================================================================

int signal_block(process_t* proc, int signum) {
    if (!proc || signum < 1 || signum >= MAX_SIGNALS) {
        return -1;
    }

    // proc->signals.blocked |= (1 << signum);
    return 0;
}

int signal_unblock(process_t* proc, int signum) {
    if (!proc || signum < 1 || signum >= MAX_SIGNALS) {
        return -1;
    }

    // proc->signals.blocked &= ~(1 << signum);

    // Handle if now pending
    // signal_handle_pending(proc);

    return 0;
}
