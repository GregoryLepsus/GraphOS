; ============================================================================
; GDT Flush - Загрузка новой GDT
; ============================================================================

[BITS 32]

global gdt_flush
extern gp

gdt_flush:
    mov eax, [esp + 4]  ; Получить аргумент (указатель на GDT)
    lgdt [eax]          ; Загрузить GDT

    ; Обновить селекторы сегментов
    mov ax, 0x10        ; 0x10 = смещение data segment в GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far jump для обновления code segment
    jmp 0x08:.flush     ; 0x08 = смещение code segment в GDT

.flush:
    ret
