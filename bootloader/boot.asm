; ============================================================================
; GraphOS Bootloader
; Простой загрузчик для x86 BIOS
; Размер: 512 байт (один сектор)
; ============================================================================

[BITS 16]               ; 16-bit режим (Real Mode)
[ORG 0x7C00]           ; BIOS загружает загрузчик по адресу 0x7C00

; ----------------------------------------------------------------------------
; Точка входа
; ----------------------------------------------------------------------------
start:
    ; Инициализация сегментных регистров
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00      ; Stack растёт вниз от загрузчика

    ; Очистка экрана и вывод приветствия
    call clear_screen
    mov si, msg_boot
    call print_string

    ; Включение A20 линии (для доступа к памяти > 1MB)
    call enable_a20

    ; Загрузка ядра с диска
    mov si, msg_loading
    call print_string
    call load_kernel

    ; Переход в защищённый режим (32-bit)
    call enter_protected_mode

    ; Этот код не должен выполниться
    jmp $

; ----------------------------------------------------------------------------
; Функция: clear_screen - Очистка экрана
; ----------------------------------------------------------------------------
clear_screen:
    pusha
    mov ah, 0x00        ; Функция: Set video mode
    mov al, 0x03        ; Mode: 80x25 text
    int 0x10
    popa
    ret

; ----------------------------------------------------------------------------
; Функция: print_string - Вывод строки на экран
; Вход: SI = указатель на строку (завершается нулём)
; ----------------------------------------------------------------------------
print_string:
    pusha
.loop:
    lodsb               ; Загрузить байт из [SI] в AL
    test al, al
    jz .done
    mov ah, 0x0E        ; Функция BIOS: teletype output
    mov bh, 0x00        ; Page number
    int 0x10
    jmp .loop
.done:
    popa
    ret

; ----------------------------------------------------------------------------
; Функция: enable_a20 - Включение линии A20
; ----------------------------------------------------------------------------
enable_a20:
    pusha

    ; Метод через BIOS
    mov ax, 0x2401
    int 0x15
    jnc .done

    ; Если BIOS не сработал, используем Fast A20
    in al, 0x92
    or al, 2
    out 0x92, al

.done:
    popa
    ret

; ----------------------------------------------------------------------------
; Функция: load_kernel - Загрузка ядра с диска в память
; ----------------------------------------------------------------------------
load_kernel:
    pusha

    mov ah, 0x02        ; Функция: Read sectors
    mov al, 20          ; Количество секторов для чтения
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Sector 2 (после bootloader)
    mov dh, 0           ; Head 0
    mov dl, 0x80        ; Drive 0 (первый HDD)
    mov bx, KERNEL_OFFSET ; Адрес загрузки
    int 0x13

    jc .disk_error

    mov si, msg_ok
    call print_string
    popa
    ret

.disk_error:
    mov si, msg_disk_error
    call print_string
    jmp $

; ----------------------------------------------------------------------------
; Функция: enter_protected_mode - Переход в защищённый режим
; ----------------------------------------------------------------------------
enter_protected_mode:
    cli                 ; Отключить прерывания

    lgdt [gdt_descriptor] ; Загрузить GDT

    ; Включить Protected Mode (установить бит PE в CR0)
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; Far jump для очистки pipeline и загрузки CS
    jmp CODE_SEG:init_pm

[BITS 32]
init_pm:
    ; Инициализация сегментных регистров в 32-bit режиме
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Установка stack pointer
    mov ebp, 0x90000
    mov esp, ebp

    ; Переход к ядру
    call KERNEL_OFFSET

    ; Не должны сюда попасть
    jmp $

; ============================================================================
; GDT (Global Descriptor Table)
; ============================================================================
gdt_start:

; Null descriptor (обязательный)
gdt_null:
    dd 0x0
    dd 0x0

; Code segment descriptor
gdt_code:
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10011010b    ; Access byte: present, ring 0, code, executable, readable
    db 11001111b    ; Flags + Limit (bits 16-19): granularity, 32-bit
    db 0x0          ; Base (bits 24-31)

; Data segment descriptor
gdt_data:
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10010010b    ; Access byte: present, ring 0, data, writable
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size
    dd gdt_start                 ; Offset

; Константы для селекторов сегментов
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; ============================================================================
; Константы и данные
; ============================================================================
KERNEL_OFFSET equ 0x1000

msg_boot        db 'GraphOS Bootloader v0.1', 0x0D, 0x0A, 0
msg_loading     db 'Loading kernel...', 0x0D, 0x0A, 0
msg_ok          db 'OK', 0x0D, 0x0A, 0
msg_disk_error  db 'Disk read error!', 0x0D, 0x0A, 0

; ============================================================================
; Boot signature (последние 2 байта должны быть 0x55AA)
; ============================================================================
times 510-($-$$) db 0
dw 0xAA55
