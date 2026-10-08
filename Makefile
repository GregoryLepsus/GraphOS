# ============================================================================
# GraphOS Makefile
# Сборка операционной системы
# ============================================================================

# Компиляторы и инструменты
ASM = nasm
CC = gcc
LD = ld
QEMU = qemu-system-i386

# Флаги компиляции
ASMFLAGS = -f elf32
CFLAGS = -m32 -std=gnu17 -nostdlib -nostartfiles -nodefaultlibs \
         -fno-builtin -fno-stack-protector \
         -isystem $(shell $(CC) -print-file-name=include) \
         -include stddef.h \
         -I./kernel/include -I./libc/include \
         -Wall -Wextra \
         -Wno-cast-function-type \
         -Wno-int-conversion \
         -Wno-incompatible-pointer-types \
         -Wno-pointer-to-int-cast \
         -Wno-unused-parameter \
         -Wno-unused-variable \
         -c
LDFLAGS = -m elf_i386 -T linker.ld

# Директории
BUILD_DIR = build
BOOT_DIR = bootloader
KERNEL_DIR = kernel

# Файлы
BOOTLOADER = $(BUILD_DIR)/boot.bin
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
OS_IMAGE = $(BUILD_DIR)/os.img

# Исходники
BOOT_SRC = $(BOOT_DIR)/boot.asm
KERNEL_ASM = $(KERNEL_DIR)/arch/x86_64/entry.asm \
             $(KERNEL_DIR)/arch/x86_64/gdt_flush.asm \
             $(KERNEL_DIR)/arch/x86_64/idt_flush.asm \
             $(KERNEL_DIR)/arch/x86_64/isr.asm \
             $(KERNEL_DIR)/arch/x86_64/switch.asm \
             $(KERNEL_DIR)/arch/x86_64/syscall_entry.asm \
             $(KERNEL_DIR)/arch/x86_64/tss_flush.asm
KERNEL_C = $(KERNEL_DIR)/core/kernel.c \
           $(KERNEL_DIR)/core/idt.c \
           $(KERNEL_DIR)/core/isr.c \
           $(KERNEL_DIR)/core/pic.c \
           $(KERNEL_DIR)/core/syscall.c \
           $(KERNEL_DIR)/core/sync.c \
           $(KERNEL_DIR)/core/tss.c \
           $(KERNEL_DIR)/drivers/timer.c \
           $(KERNEL_DIR)/drivers/keyboard.c \
           $(KERNEL_DIR)/drivers/ide.c \
           $(KERNEL_DIR)/drivers/pci.c \
           $(KERNEL_DIR)/drivers/e1000.c \
           $(KERNEL_DIR)/drivers/vga.c \
           $(KERNEL_DIR)/drivers/mouse.c \
           $(KERNEL_DIR)/mm/pmm.c \
           $(KERNEL_DIR)/mm/paging.c \
           $(KERNEL_DIR)/mm/heap.c \
           $(KERNEL_DIR)/process/process.c \
           $(KERNEL_DIR)/fs/vfs.c \
           $(KERNEL_DIR)/fs/initrd.c \
           $(KERNEL_DIR)/fs/elf.c \
           $(KERNEL_DIR)/shell/parser.c \
           $(KERNEL_DIR)/shell/executor.c \
           $(KERNEL_DIR)/net/ethernet.c \
           $(KERNEL_DIR)/net/arp.c \
           $(KERNEL_DIR)/net/ip.c \
           $(KERNEL_DIR)/net/icmp.c \
           $(KERNEL_DIR)/net/udp.c \
           $(KERNEL_DIR)/net/tcp.c \
           $(KERNEL_DIR)/net/socket.c \
           $(KERNEL_DIR)/net/dns.c \
           $(KERNEL_DIR)/net/netutils.c \
           $(KERNEL_DIR)/graphics/framebuffer.c \
           $(KERNEL_DIR)/graphics/font.c \
           $(KERNEL_DIR)/graphics/window.c \
           $(KERNEL_DIR)/gui/widget.c \
           $(KERNEL_DIR)/hal/hal_core.c \
           $(KERNEL_DIR)/drivers/acpi/acpi.c \
           $(KERNEL_DIR)/drivers/dma/dma.c \
           $(KERNEL_DIR)/drivers/storage/ahci.c \
           $(KERNEL_DIR)/drivers/audio/ac97.c \
           $(KERNEL_DIR)/drivers/usb/uhci.c \
           $(KERNEL_DIR)/drivers/usb/usb_hid.c \
           $(KERNEL_DIR)/drivers/usb/usb_storage.c \
           $(KERNEL_DIR)/smp/apic.c \
           $(KERNEL_DIR)/smp/smp.c \
           $(KERNEL_DIR)/process/thread.c \
           $(KERNEL_DIR)/core/sync_advanced.c \
           $(KERNEL_DIR)/ipc/message.c \
           $(KERNEL_DIR)/ipc/shm.c \
           $(KERNEL_DIR)/ipc/pipe.c \
           $(KERNEL_DIR)/process/scheduler_advanced.c \
           $(KERNEL_DIR)/security/security.c \
           $(KERNEL_DIR)/process/signal.c \
           $(KERNEL_DIR)/fs/vfs_cache.c \
           $(KERNEL_DIR)/drivers/device_manager.c \
           $(KERNEL_DIR)/core/timer_advanced.c \
           $(KERNEL_DIR)/fs/vfs_links.c \
           $(KERNEL_DIR)/core/profiler.c \
           $(KERNEL_DIR)/mm/memory_optimizer.c

# Объектные файлы
KERNEL_ASM_OBJ = $(patsubst $(KERNEL_DIR)/%.asm,$(BUILD_DIR)/%.o,$(KERNEL_ASM))
KERNEL_C_OBJ = $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/%.o,$(KERNEL_C))
KERNEL_OBJ = $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJ)

# ============================================================================
# Основные цели
# ============================================================================

.PHONY: all clean run debug dirs

all: dirs $(OS_IMAGE)

dirs:
	@mkdir -p $(BUILD_DIR)/arch/x86_64
	@mkdir -p $(BUILD_DIR)/core
	@mkdir -p $(BUILD_DIR)/drivers
	@mkdir -p $(BUILD_DIR)/process
	@mkdir -p $(BUILD_DIR)/mm
	@mkdir -p $(BUILD_DIR)/fs

# Сборка образа ОС
$(OS_IMAGE): $(BOOTLOADER) $(KERNEL_BIN)
	@echo "Creating OS image..."
	@cat $(BOOTLOADER) $(KERNEL_BIN) > $(OS_IMAGE)
	@# Дополняем до размера дискеты (1.44 MB)
	@truncate -s 1440K $(OS_IMAGE)
	@echo "OS image created: $(OS_IMAGE)"

# Сборка загрузчика
$(BOOTLOADER): $(BOOT_SRC)
	@echo "Building bootloader..."
	@$(ASM) -f bin $(BOOT_SRC) -o $(BOOTLOADER)

# Сборка ядра
$(KERNEL_BIN): $(KERNEL_OBJ)
	@echo "Linking kernel..."
	@$(LD) $(LDFLAGS) -o $(KERNEL_BIN) $(KERNEL_OBJ)

# Компиляция assembly файлов
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.asm
	@mkdir -p $(dir $@)
	@echo "Assembling $<..."
	@$(ASM) $(ASMFLAGS) $< -o $@

# Компиляция C файлов
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

# ============================================================================
# Запуск и отладка
# ============================================================================

run: all
	@echo "Starting QEMU..."
	$(QEMU) -drive format=raw,file=$(OS_IMAGE)

debug: all
	@echo "Starting QEMU with GDB server..."
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) -s -S &
	@echo "Waiting for GDB connection on localhost:1234"
	@echo "Use: gdb -ex 'target remote localhost:1234' -ex 'break kernel_main'"

# ============================================================================
# Очистка
# ============================================================================

clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

# ============================================================================
# Информация
# ============================================================================

info:
	@echo "GraphOS Build System"
	@echo "===================="
	@echo "Targets:"
	@echo "  all     - Build complete OS image"
	@echo "  run     - Build and run in QEMU"
	@echo "  debug   - Build and start GDB server"
	@echo "  clean   - Remove build artifacts"
	@echo "  info    - Show this help"
	@echo ""
	@echo "Tools:"
	@echo "  ASM:    $(ASM)"
	@echo "  CC:     $(CC)"
	@echo "  LD:     $(LD)"
	@echo "  QEMU:   $(QEMU)"
