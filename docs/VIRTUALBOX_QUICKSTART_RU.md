# Быстрая настройка GraphOS в VirtualBox

## Системные требования

### Компьютер (хост)
- **ОС:** Windows, macOS или Linux
- **ОЗУ:** 4 ГБ и более (рекомендуется)
- **Диск:** 100 МБ свободного места
- **VirtualBox:** версия 6.1 или выше

### Виртуальная машина
- **ОЗУ:** 512 МБ минимум, 1 ГБ рекомендуется
- **Диск:** 50 МБ минимум
- **ЦПУ:** 1 ядро минимум, 2 рекомендуется
- **Графика:** VGA или VMSVGA

---

## Шаг 1: Установка VirtualBox

1. Загрузите VirtualBox с сайта https://www.virtualbox.org/wiki/Downloads
2. Установите версию 6.1 или выше для вашей ОС

---

## Шаг 2: Создание виртуальной машины

1. Откройте VirtualBox
2. Нажмите кнопку **"New"** (Создать)
3. Введите параметры:
   - **Name:** GraphOS
   - **Type:** Other (Другое)
   - **Version:** Other/Unknown (32-bit)
4. **Память:** выберите 1024 МБ (1 ГБ)
5. **Жёсткий диск:** выберите **"Do not add a virtual hard disk"** (Не добавлять)
6. Нажмите **"Create"** (Создать)

---

## Шаг 3: Основные настройки

Нажмите правой кнопкой на машину GraphOS → **Settings** (Параметры)

### Система

**Motherboard (Материнская плата):**
- Boot Order (Порядок загрузки): Floppy (первым)
- Chipset (Чипсет): PIIX3
- I/O APIC: включить
- EFI: отключить

**Processor (Процессор):**
- Процессоры: 1-2
- PAE/NX: включить

**Acceleration (Ускорение):**
- VT-x/AMD-V: включить
- Nested Paging: включить

### Дисплей

**Screen (Экран):**
- Video Memory: 32 МБ
- Graphics Controller: **VGA** (важно!)
- 3D/2D Acceleration: отключить

### Хранилище

**Floppy Controller:**
1. Нажмите на "Floppy Device" (Устройство дискеты)
2. Нажмите на иконку диска справа
3. Выберите **"Choose/Create a Virtual Floppy Disk"**
4. Укажите файл **floppy.img** (созданный после сборки)

---

## Шаг 4: Сборка GraphOS

Перед запуском виртуальной машины соберите образ:

```bash
cd GraphOS
make clean
make all
```

Это создаст файл **floppy.img**

---

## Шаг 5: Запуск

1. Выберите GraphOS машину
2. Нажмите **"Start"** (Запустить)
3. Машина должна загрузиться из floppy.img

---

## Ожидаемый результат загрузки

```
Welcome to GraphOS v2.0
Initializing...
[OK] GDT
[OK] IDT
[OK] Timer
[OK] Keyboard
[OK] Memory
[OK] Paging
[OK] GUI

GraphOS Shell >
```

---

## Решение проблем

| Проблема | Решение |
|----------|---------|
| Чёрный экран | Проверьте floppy.img подключен, пересоберите образ |
| Kernel Panic | Увеличьте ОЗУ до 1 ГБ, отключите EFI |
| Искажённый экран | Установите Graphics Controller на VGA |
| Клавиатура не работает | Нажмите внутри окна машины для захвата фокуса |
| Медленное выполнение | Включите VT-x/AMD-V, выделите 2 ядра и 1 ГБ ОЗУ |

---

## Команды управления (Windows/Linux/macOS)

```bash
# Запуск машины
VBoxManage startvm GraphOS

# Остановка машины
VBoxManage controlvm GraphOS poweroff

# Информация о машине
VBoxManage showvminfo GraphOS
```

---

## Оптимальные параметры производительности

```
ОЗУ:              1024 МБ
ЦПУ:              2 ядра
Video Memory:     32 МБ
Graphics:         VGA
VT-x:             включить
Nested Paging:    включить
I/O APIC:         включить
EFI:              отключить
```

---

**Дата обновления:** 2026-10-08  
**Версия:** 2.0  
**Протестировано:** VirtualBox 7.0, 6.1
