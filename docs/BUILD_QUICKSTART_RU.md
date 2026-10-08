# Быстрый старт - Сборка GraphOS

## Требуемые инструменты

### Обязательные
- **GCC:** кросс-компилятор для i686-elf (или gcc -m32)
- **NASM:** ассемблер (версия 2.14+)
- **GNU Make:** 4.0+
- **GNU LD:** компоновщик

### Рекомендуемые
- **QEMU:** для тестирования (qemu-system-i386)
- **GDB:** для отладки

---

## Установка зависимостей

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential nasm qemu-system-x86
sudo apt-get install gcc-multilib
```

### macOS
```bash
brew install i686-elf-gcc nasm qemu
```

### Windows (WSL)
```bash
# Внутри WSL Ubuntu:
sudo apt-get update
sudo apt-get install build-essential nasm qemu-system-x86
```

---

## Быстрая сборка

```bash
# Перейдите в каталог GraphOS
cd GraphOS

# Очистить и собрать всё
make clean
make all

# Запустить в QEMU
make run

# Или отладить с GDB
make debug
```

---

## Что происходит при `make all`

1. **Bootloader** - компилируется в boot.bin
2. **Kernel** - компилируется C и ассемблер → kernel.elf → kernel.bin
3. **Userland** - программы пользователя собираются
4. **InitRD** - образ инициализации с программами
5. **Floppy Image** - создаётся floppy.img (1.4 МБ)

---

## Цели сборки

```bash
make kernel       # Только ядро
make userland     # Только программы пользователя
make iso          # Загружаемый ISO образ
make run          # Собрать и запустить в QEMU
make debug        # Запустить с отладкой GDB
make clean        # Удалить все артефакты
```

---

## Структура проекта

```
GraphOS/
├── bootloader/    # BIOS загрузчик
├── kernel/        # ОС ядро
├── userland/      # Программы пользователя
└── Makefile       # Главный файл сборки
```

---

## Проверка после сборки

Файл **floppy.img** должен быть ~1.4 МБ:

```bash
ls -lh floppy.img
```

Образ готов для:
- Запуска в QEMU: `qemu-system-i386 -fda floppy.img`
- Использования в VirtualBox
- Загрузки на реальное оборудование

---

## Решение проблем при сборке

| Ошибка | Решение |
|--------|---------|
| i686-elf-gcc: command not found | Используйте `gcc -m32` или установите кросс-компилятор |
| undefined reference | Проверьте linker script (kernel/link.ld) |
| NASM not found | Установите NASM: `apt-get install nasm` |
| make: command not found | Установите GNU Make |

---

## Параметры компилятора

### Ядро
```makefile
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector
CFLAGS += -Wall -Wextra -O2 -Iinclude
```

### Userland
```makefile
CFLAGS = -m32 -ffreestanding -nostdlib -Wall -Wextra -O2
```

---

## Сборка с оптимизацией

```bash
# Быстрая сборка (параллельно)
make -j$(nproc) all

# С размер-оптимизацией
make CFLAGS="-Os" all

# С отладочными символами
make CFLAGS="-g -O0" all
```

---

## Тестирование

```bash
# Запуск в QEMU
make run

# Ожидаемый вывод:
# Welcome to GraphOS v2.0
# GraphOS Shell >

# Командный тест
make test
```

---

## Развёртывание

### Загрузочный USB (Linux/macOS)

⚠️ **Будет стёрт весь диск!**

```bash
sudo dd if=floppy.img of=/dev/sdX bs=4M
sudo sync
```

Замените `/dev/sdX` на ваше USB устройство!

### Образ ISO

```bash
make iso
# Создаёт graphos.iso для VirtualBox/VMware
```

---

## Производительность

### Используйте кэш

```bash
export CC="ccache gcc"
make clean
make all
```

### Параллельная компиляция

```bash
make -j4 all    # 4 потока
make -j$(nproc) all  # Все доступные ядра
```

---

## Справка

Для дополнительной информации:
- Полная документация: `docs/BUILD.md`
- OSDev Wiki: https://wiki.osdev.org
- Получить помощь в проекте

---

**Дата обновления:** 2026-10-08  
**Версия:** 2.0  
**Проверено на:** GCC 9+, NASM 2.14+, GNU Make 4.0+
