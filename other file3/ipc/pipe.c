// ============================================================================
// Enhanced Pipe Implementation
// Named pipes (FIFOs) with buffering and non-blocking I/O
// ============================================================================

#include "../include/pipe.h"
#include "../include/heap.h"

// ============================================================================
// Pipe Management
// ============================================================================

pipe_t* pipe_create(void) {
    pipe_t* pipe = (pipe_t*)kmalloc(sizeof(pipe_t));
    if (!pipe) return NULL;

    pipe->read_pos = 0;
    pipe->write_pos = 0;
    pipe->count = 0;

    mutex_init(&pipe->lock);
    semaphore_init(&pipe->not_empty, 0);
    semaphore_init(&pipe->not_full, 4096);

    return pipe;
}

void pipe_close(pipe_t* pipe) {
    if (!pipe) return;
    kfree(pipe);
}

// ============================================================================
// Pipe I/O Operations
// ============================================================================

int pipe_write(pipe_t* pipe, const void* data, uint32_t size) {
    if (!pipe || !data || size == 0) return -1;

    const uint8_t* src = (const uint8_t*)data;
    uint32_t written = 0;

    while (written < size) {
        // Wait for space
        semaphore_wait(&pipe->not_full);

        mutex_lock(&pipe->lock);

        // Write one byte
        pipe->buffer[pipe->write_pos] = src[written];
        pipe->write_pos = (pipe->write_pos + 1) % 4096;
        pipe->count++;
        written++;

        mutex_unlock(&pipe->lock);

        // Signal data available
        semaphore_signal(&pipe->not_empty);
    }

    return written;
}

int pipe_read(pipe_t* pipe, void* data, uint32_t size) {
    if (!pipe || !data || size == 0) return -1;

    uint8_t* dst = (uint8_t*)data;
    uint32_t read = 0;

    while (read < size) {
        // Wait for data
        semaphore_wait(&pipe->not_empty);

        mutex_lock(&pipe->lock);

        // Read one byte
        dst[read] = pipe->buffer[pipe->read_pos];
        pipe->read_pos = (pipe->read_pos + 1) % 4096;
        pipe->count--;
        read++;

        mutex_unlock(&pipe->lock);

        // Signal space available
        semaphore_signal(&pipe->not_full);
    }

    return read;
}
