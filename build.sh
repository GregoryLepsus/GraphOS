#!/bin/bash
# GraphOS Build Script

set -e

echo "=== GraphOS Build System ==="
echo ""

# Создать директории
mkdir -p build/arch/x86_64
mkdir -p build/core

# Собрать bootloader
echo "[1/4] Building bootloader..."
nasm -f bin bootloader/boot.asm -o build/boot.bin

# Собрать assembly файлы ядра
echo "[2/4] Assembling kernel entry..."
nasm -f elf32 kernel/arch/x86_64/entry.asm -o build/arch/x86_64/entry.o
nasm -f elf32 kernel/arch/x86_64/gdt_flush.asm -o build/arch/x86_64/gdt_flush.o

# Собрать C файлы ядра
echo "[3/4] Compiling kernel..."
gcc -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
    -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c \
    -Ikernel/include kernel/core/kernel.c -o build/core/kernel.o

# Линковка ядра
echo "[4/4] Linking kernel..."
ld -m elf_i386 -T linker.ld -o build/kernel.bin \
   build/arch/x86_64/entry.o \
   build/arch/x86_64/gdt_flush.o \
   build/core/kernel.o

# Создать образ диска
echo "Creating OS image..."
cat build/boot.bin build/kernel.bin > build/os.img
truncate -s 1440K build/os.img

echo ""
echo "=== Build Complete ==="
echo "OS image: build/os.img"
echo ""
echo "To run: qemu-system-i386 -drive format=raw,file=build/os.img"
