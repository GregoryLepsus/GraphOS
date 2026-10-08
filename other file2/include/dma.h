#ifndef DMA_H
#define DMA_H

#include <stdint.h>

// DMA Direction
#define DMA_TO_DEVICE    0
#define DMA_FROM_DEVICE  1
#define DMA_BIDIRECTIONAL 2

// DMA Transfer Structure
typedef struct {
    uint8_t channel;
    void* src_addr;
    void* dst_addr;
    uint32_t size;
    uint8_t direction;
    uint8_t mode;
    void (*callback)(void* data);
    void* callback_data;
} dma_transfer_t;

// DMA Modes
#define DMA_MODE_SINGLE    0
#define DMA_MODE_BLOCK     1
#define DMA_MODE_CASCADE   2

// DMA Status
#define DMA_STATUS_IDLE      0
#define DMA_STATUS_ACTIVE    1
#define DMA_STATUS_COMPLETE  2
#define DMA_STATUS_ERROR     3

// Functions
void dma_init(void);
int dma_allocate_channel(void);
void dma_free_channel(uint8_t channel);
int dma_setup_transfer(dma_transfer_t* transfer);
int dma_start_transfer(uint8_t channel);
void dma_stop_transfer(uint8_t channel);
int dma_get_status(uint8_t channel);
void dma_wait_completion(uint8_t channel);
void* dma_alloc_buffer(uint32_t size);
void dma_free_buffer(void* buffer);

#endif // DMA_H
