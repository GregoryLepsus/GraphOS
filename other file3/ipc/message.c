// ============================================================================
// Message Passing IPC
// Inter-process communication via message queues
// ============================================================================

#include "../include/message.h"
#include "../include/heap.h"
#include "../include/thread.h"

static uint32_t next_queue_id = 1;

// ============================================================================
// Message Queue Management
// ============================================================================

msgqueue_t* msgqueue_create(uint32_t max_messages) {
    msgqueue_t* queue = (msgqueue_t*)kmalloc(sizeof(msgqueue_t));
    if (!queue) return NULL;

    queue->id = next_queue_id++;
    queue->max_messages = max_messages;
    queue->count = 0;
    queue->head = NULL;
    queue->tail = NULL;

    mutex_init(&queue->lock);
    semaphore_init(&queue->not_empty, 0);
    semaphore_init(&queue->not_full, max_messages);

    return queue;
}

void msgqueue_destroy(msgqueue_t* queue) {
    if (!queue) return;

    mutex_lock(&queue->lock);

    // Free all messages
    message_node_t* node = queue->head;
    while (node) {
        message_node_t* next = node->next;
        kfree(node);
        node = next;
    }

    mutex_unlock(&queue->lock);
    kfree(queue);
}

// ============================================================================
// Message Operations
// ============================================================================

int msg_send(msgqueue_t* queue, message_t* msg, uint32_t timeout) {
    if (!queue || !msg) return -1;

    // Wait for space
    semaphore_wait(&queue->not_full);

    mutex_lock(&queue->lock);

    // Create message node
    message_node_t* node = (message_node_t*)kmalloc(sizeof(message_node_t));
    if (!node) {
        mutex_unlock(&queue->lock);
        return -1;
    }

    // Copy message
    node->msg = *msg;
    node->next = NULL;

    // Add to queue
    if (queue->tail) {
        queue->tail->next = node;
    } else {
        queue->head = node;
    }
    queue->tail = node;
    queue->count++;

    mutex_unlock(&queue->lock);

    // Signal not empty
    semaphore_signal(&queue->not_empty);

    return 0;
}

int msg_receive(msgqueue_t* queue, message_t* msg, uint32_t timeout) {
    if (!queue || !msg) return -1;

    // Wait for message
    semaphore_wait(&queue->not_empty);

    mutex_lock(&queue->lock);

    if (!queue->head) {
        mutex_unlock(&queue->lock);
        return -1;
    }

    // Get first message
    message_node_t* node = queue->head;
    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    queue->count--;

    // Copy message
    *msg = node->msg;

    mutex_unlock(&queue->lock);

    // Free node
    kfree(node);

    // Signal not full
    semaphore_signal(&queue->not_full);

    return 0;
}

int msg_try_receive(msgqueue_t* queue, message_t* msg) {
    if (!queue || !msg) return -1;

    mutex_lock(&queue->lock);

    if (!queue->head) {
        mutex_unlock(&queue->lock);
        return -1;
    }

    // Get first message
    message_node_t* node = queue->head;
    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    queue->count--;

    // Copy message
    *msg = node->msg;

    mutex_unlock(&queue->lock);

    // Free node
    kfree(node);

    // Signal not full
    semaphore_signal(&queue->not_full);

    return 0;
}

uint32_t msgqueue_count(msgqueue_t* queue) {
    if (!queue) return 0;

    mutex_lock(&queue->lock);
    uint32_t count = queue->count;
    mutex_unlock(&queue->lock);

    return count;
}
