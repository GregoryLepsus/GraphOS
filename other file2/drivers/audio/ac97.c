// ============================================================================
// AC97 Audio Driver
// Audio playback support via AC97 codec
// ============================================================================

#include "../include/audio.h"
#include "../include/pci.h"
#include "../include/heap.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// Port I/O
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outl(uint16_t port, uint32_t value) {
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t inl(uint16_t port) {
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// AC97 registers
#define AC97_RESET              0x00
#define AC97_MASTER_VOLUME      0x02
#define AC97_PCM_OUT_VOLUME     0x18
#define AC97_EXTENDED_AUDIO_ID  0x28
#define AC97_EXTENDED_AUDIO_STA 0x2A

// Native Audio Mixer (NAM) registers
static uint16_t nam_base = 0;

// Native Audio Bus Master (NABM) registers
static uint16_t nabm_base = 0;

// Buffer Descriptor List Entry
typedef struct {
    uint32_t buffer_addr;
    uint16_t samples;
    uint16_t flags;
} __attribute__((packed)) bdl_entry_t;

#define BDL_IOC  0x8000  // Interrupt on completion

// Audio state
static int audio_initialized = 0;
static int audio_state = AUDIO_STATE_STOPPED;
static uint8_t current_volume = 75;
static audio_format_t current_format;
static bdl_entry_t* bdl = NULL;
static void* audio_buffer = NULL;

// ============================================================================
// AC97 Codec Functions
// ============================================================================

static void ac97_write_register(uint8_t reg, uint16_t value) {
    outw(nam_base + reg, value);
}

static uint16_t ac97_read_register(uint8_t reg) {
    return inw(nam_base + reg);
}

// ============================================================================
// Initialization
// ============================================================================

void audio_init(void) {
    terminal_write_line("[Audio] Initializing AC97 audio...");

    // Find AC97 device on PCI bus
    pci_device_t* audio_dev = pci_find_device(0x8086, 0x2415);  // Intel ICH

    if (!audio_dev) {
        terminal_write_line("[Audio] AC97 device not found");
        return;
    }

    terminal_write("[Audio] Found AC97 device at PCI ");
    terminal_write_hex(audio_dev->bus);
    terminal_write(":");
    terminal_write_hex(audio_dev->device);
    terminal_write_line("");

    // Get BAR addresses
    nam_base = pci_read_bar(audio_dev, 0) & 0xFFFE;   // NAM (mixer)
    nabm_base = pci_read_bar(audio_dev, 1) & 0xFFFE;  // NABM (bus master)

    terminal_write("[Audio] NAM base: 0x");
    terminal_write_hex(nam_base);
    terminal_write_line("");
    terminal_write("[Audio] NABM base: 0x");
    terminal_write_hex(nabm_base);
    terminal_write_line("");

    // Reset codec
    ac97_write_register(AC97_RESET, 0);

    // Wait for codec ready
    for (int i = 0; i < 1000; i++) {
        if (ac97_read_register(AC97_RESET) & 0x8000) {
            break;
        }
    }

    // Set default format
    current_format.sample_rate = AUDIO_RATE_44100;
    current_format.channels = 2;
    current_format.bits_per_sample = 16;
    current_format.buffer_size = 4096;

    // Set default volume
    audio_set_volume(current_volume);

    // Allocate buffer descriptor list
    bdl = (bdl_entry_t*)kmalloc(32 * sizeof(bdl_entry_t));

    audio_initialized = 1;
    audio_state = AUDIO_STATE_STOPPED;

    terminal_write_line("[Audio] AC97 audio initialized");
}

// ============================================================================
// Audio Control
// ============================================================================

int audio_open(audio_format_t* format) {
    if (!audio_initialized) {
        return -1;
    }

    if (format) {
        current_format.sample_rate = format->sample_rate;
        current_format.channels = format->channels;
        current_format.bits_per_sample = format->bits_per_sample;
        current_format.buffer_size = format->buffer_size;
    }

    // Allocate audio buffer
    if (audio_buffer) {
        kfree(audio_buffer);
    }
    audio_buffer = kmalloc(current_format.buffer_size);

    return 0;
}

void audio_close(void) {
    audio_stop();

    if (audio_buffer) {
        kfree(audio_buffer);
        audio_buffer = NULL;
    }
}

int audio_play_buffer(void* buffer, uint32_t size) {
    if (!audio_initialized || !buffer) {
        return -1;
    }

    // Setup buffer descriptor
    bdl[0].buffer_addr = (uint32_t)buffer;
    bdl[0].samples = size / 4;  // Samples (16-bit stereo)
    bdl[0].flags = BDL_IOC;

    // Set BDL address
    outl(nabm_base + 0x10, (uint32_t)bdl);

    // Set last valid index
    outb(nabm_base + 0x15, 0);

    // Start playback
    outb(nabm_base + 0x1B, 0x01);

    audio_state = AUDIO_STATE_PLAYING;
    return 0;
}

int audio_write(void* data, uint32_t size) {
    if (!audio_initialized || !audio_buffer) {
        return -1;
    }

    // Copy data to audio buffer
    uint8_t* src = (uint8_t*)data;
    uint8_t* dst = (uint8_t*)audio_buffer;
    for (uint32_t i = 0; i < size && i < current_format.buffer_size; i++) {
        dst[i] = src[i];
    }

    return audio_play_buffer(audio_buffer, size);
}

// ============================================================================
// Volume Control
// ============================================================================

void audio_set_volume(uint8_t volume) {
    if (!audio_initialized) {
        return;
    }

    if (volume > 100) {
        volume = 100;
    }

    current_volume = volume;

    // Convert 0-100 to AC97 volume (0-31, inverted)
    uint8_t ac97_vol = 31 - (volume * 31 / 100);

    // Set master volume (left and right)
    uint16_t vol_reg = (ac97_vol << 8) | ac97_vol;
    ac97_write_register(AC97_MASTER_VOLUME, vol_reg);

    // Set PCM out volume
    ac97_write_register(AC97_PCM_OUT_VOLUME, vol_reg);
}

uint8_t audio_get_volume(void) {
    return current_volume;
}

// ============================================================================
// Playback Control
// ============================================================================

void audio_pause(void) {
    if (audio_state == AUDIO_STATE_PLAYING) {
        outb(nabm_base + 0x1B, 0x00);  // Stop
        audio_state = AUDIO_STATE_PAUSED;
    }
}

void audio_resume(void) {
    if (audio_state == AUDIO_STATE_PAUSED) {
        outb(nabm_base + 0x1B, 0x01);  // Start
        audio_state = AUDIO_STATE_PLAYING;
    }
}

void audio_stop(void) {
    if (audio_state != AUDIO_STATE_STOPPED) {
        outb(nabm_base + 0x1B, 0x00);  // Stop
        audio_state = AUDIO_STATE_STOPPED;
    }
}

int audio_get_state(void) {
    return audio_state;
}

// ============================================================================
// Capabilities
// ============================================================================

void audio_get_caps(audio_caps_t* caps) {
    if (!caps) {
        return;
    }

    caps->max_sample_rate = AUDIO_RATE_48000;
    caps->min_sample_rate = AUDIO_RATE_8000;
    caps->max_channels = 2;
    caps->supports_16bit = 1;
    caps->buffer_size = 65536;
}
