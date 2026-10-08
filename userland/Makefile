# ============================================================================
# Userland Programs Makefile
# Сборка пользовательских программ для GraphOS
# ============================================================================

# Компиляторы
ASM = nasm
CC = gcc
LD = ld
AR = ar

# Флаги компиляции
ASMFLAGS = -f elf32
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c \
         -Iinclude -fno-pie
LDFLAGS = -m elf_i386 -nostdlib

# Директории
LIBC_DIR = libc
BUILD_DIR = build

# Исходники libc
LIBC_C = $(LIBC_DIR)/syscall.c \
         $(LIBC_DIR)/string.c \
         $(LIBC_DIR)/stdio.c \
         $(LIBC_DIR)/stdlib.c

LIBC_ASM = $(LIBC_DIR)/_start.asm

# Объектные файлы libc
LIBC_C_OBJ = $(patsubst $(LIBC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIBC_C))
LIBC_ASM_OBJ = $(BUILD_DIR)/_start.o

# Статическая библиотека
LIBC_A = $(BUILD_DIR)/libc.a

# Пользовательские программы
PROGRAMS = hello cat fork_test echo ls

# ============================================================================
# Основные цели
# ============================================================================

.PHONY: all clean dirs programs

all: dirs $(LIBC_A) programs

programs: $(PROGRAMS)

dirs:
	@mkdir -p $(BUILD_DIR)

# ============================================================================
# Сборка libc.a
# ============================================================================

$(LIBC_A): $(LIBC_C_OBJ) $(LIBC_ASM_OBJ)
	@echo "Creating static library: $@"
	$(AR) rcs $@ $^

# Компиляция C файлов libc
$(BUILD_DIR)/%.o: $(LIBC_DIR)/%.c
	@echo "Compiling: $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

# Компиляция ASM файлов libc
$(BUILD_DIR)/_start.o: $(LIBC_DIR)/_start.asm
	@echo "Assembling: $<"
	$(ASM) $(ASMFLAGS) $< -o $@

# ============================================================================
# Сборка пользовательских программ
# ============================================================================

# Правило для каждой программы
$(PROGRAMS): %: %.c $(LIBC_A)
	@echo "Building program: $@"
	$(CC) $(CFLAGS) $< -o $(BUILD_DIR)/$@.o
	$(LD) $(LDFLAGS) -e _start $(LIBC_ASM_OBJ) $(BUILD_DIR)/$@.o $(LIBC_A) -o $(BUILD_DIR)/$@
	@echo "  [OK] $@ built successfully"

# ============================================================================
# Очистка
# ============================================================================

clean:
	@echo "Cleaning userland build files..."
	rm -rf $(BUILD_DIR)
	@echo "  [OK] Clean complete"

# ============================================================================
# Вывод информации
# ============================================================================

info:
	@echo "Userland Build System"
	@echo "====================="
	@echo "Programs: $(PROGRAMS)"
	@echo "Build directory: $(BUILD_DIR)"
	@echo "libc sources: $(LIBC_C)"
	@echo ""
	@echo "Targets:"
	@echo "  make all      - Build everything"
	@echo "  make programs - Build all programs"
	@echo "  make clean    - Clean build files"
	@echo "  make info     - Show this information"
