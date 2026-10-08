; ============================================================================
; Kernel Entry Point
; Точка входа в ядро после загрузки bootloader'ом
; ============================================================================

[BITS 32]
[EXTERN kernel_main]

section .text
    global _start

_start:
    ; Очистка экрана в режиме VGA text mode
    mov edi, 0xB8000    ; VGA text buffer
    mov ecx, 2000       ; 80x25 = 2000 символов
    mov ax, 0x0F20      ; Белый на черном, пробел
    rep stosw           ; Заполнить буфер

    ; Вывод приветственного сообщения
    mov edi, 0xB8000
    mov esi, welcome_msg
    call print_string_32

    ; Вызов функции kernel_main из C
    call kernel_main

    ; Остановка (если kernel_main вернётся)
    cli
    hlt
    jmp $

; ----------------------------------------------------------------------------
; Функция: print_string_32 - Вывод строки в VGA text mode
; ESI = указатель на строку
; EDI = адрес в VGA буфере
; ----------------------------------------------------------------------------
print_string_32:
    push eax
    push edi
    push esi

.loop:
    lodsb               ; Загрузить символ из [ESI]
    test al, al
    jz .done

    mov ah, 0x0F        ; Атрибут: белый на черном
    stosw               ; Записать в VGA буфер
    jmp .loop

.done:
    pop esi
    pop edi
    pop eax
    ret

section .data
    welcome_msg db 'GraphOS Kernel Starting...', 0
