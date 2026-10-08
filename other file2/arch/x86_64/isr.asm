; ============================================================================
; ISR (Interrupt Service Routines) - Обработчики прерываний
; Stubs для всех CPU exceptions и IRQ
; ============================================================================

[BITS 32]

; Экспорт всех ISR для использования в C
global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
global irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7
global irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15

; Внешняя функция обработчика в C
extern isr_handler
extern irq_handler

; ============================================================================
; Макрос для ISR без кода ошибки
; ============================================================================
%macro ISR_NOERRCODE 1
isr%1:
    cli                 ; Отключить прерывания
    push byte 0         ; Dummy error code
    push byte %1        ; Номер прерывания
    jmp isr_common_stub
%endmacro

; ============================================================================
; Макрос для ISR с кодом ошибки
; ============================================================================
%macro ISR_ERRCODE 1
isr%1:
    cli                 ; Отключить прерывания
    ; CPU уже положил код ошибки на стек
    push byte %1        ; Номер прерывания
    jmp isr_common_stub
%endmacro

; ============================================================================
; Макрос для IRQ
; ============================================================================
%macro IRQ 2
irq%1:
    cli
    push byte 0         ; Dummy error code
    push byte %2        ; Номер IRQ
    jmp irq_common_stub
%endmacro

; ============================================================================
; CPU Exceptions (0-31)
; ============================================================================
ISR_NOERRCODE 0   ; Division By Zero
ISR_NOERRCODE 1   ; Debug
ISR_NOERRCODE 2   ; Non Maskable Interrupt
ISR_NOERRCODE 3   ; Breakpoint
ISR_NOERRCODE 4   ; Into Detected Overflow
ISR_NOERRCODE 5   ; Out of Bounds
ISR_NOERRCODE 6   ; Invalid Opcode
ISR_NOERRCODE 7   ; No Coprocessor
ISR_ERRCODE   8   ; Double Fault (with error code)
ISR_NOERRCODE 9   ; Coprocessor Segment Overrun
ISR_ERRCODE   10  ; Bad TSS (with error code)
ISR_ERRCODE   11  ; Segment Not Present (with error code)
ISR_ERRCODE   12  ; Stack Fault (with error code)
ISR_ERRCODE   13  ; General Protection Fault (with error code)
ISR_ERRCODE   14  ; Page Fault (with error code)
ISR_NOERRCODE 15  ; Unknown Interrupt
ISR_NOERRCODE 16  ; Coprocessor Fault
ISR_NOERRCODE 17  ; Alignment Check
ISR_NOERRCODE 18  ; Machine Check
ISR_NOERRCODE 19  ; Reserved
ISR_NOERRCODE 20  ; Reserved
ISR_NOERRCODE 21  ; Reserved
ISR_NOERRCODE 22  ; Reserved
ISR_NOERRCODE 23  ; Reserved
ISR_NOERRCODE 24  ; Reserved
ISR_NOERRCODE 25  ; Reserved
ISR_NOERRCODE 26  ; Reserved
ISR_NOERRCODE 27  ; Reserved
ISR_NOERRCODE 28  ; Reserved
ISR_NOERRCODE 29  ; Reserved
ISR_NOERRCODE 30  ; Reserved
ISR_NOERRCODE 31  ; Reserved

; ============================================================================
; IRQ (32-47) - Hardware Interrupts
; ============================================================================
IRQ 0,  32   ; Timer
IRQ 1,  33   ; Keyboard
IRQ 2,  34   ; Cascade
IRQ 3,  35   ; COM2
IRQ 4,  36   ; COM1
IRQ 5,  37   ; LPT2
IRQ 6,  38   ; Floppy
IRQ 7,  39   ; LPT1
IRQ 8,  40   ; RTC
IRQ 9,  41   ; Free
IRQ 10, 42   ; Free
IRQ 11, 43   ; Free
IRQ 12, 44   ; PS2 Mouse
IRQ 13, 45   ; FPU
IRQ 14, 46   ; Primary ATA
IRQ 15, 47   ; Secondary ATA

; ============================================================================
; Общий обработчик для CPU exceptions
; ============================================================================
isr_common_stub:
    pusha           ; Сохранить все регистры общего назначения

    mov ax, ds      ; Сохранить data segment
    push eax

    mov ax, 0x10    ; Загрузить kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler ; Вызвать C обработчик

    pop eax         ; Восстановить data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa            ; Восстановить регистры
    add esp, 8      ; Убрать int_no и err_code
    sti             ; Включить прерывания
    iret            ; Вернуться из прерывания

; ============================================================================
; Общий обработчик для IRQ
; ============================================================================
irq_common_stub:
    pusha

    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call irq_handler ; Вызвать C обработчик IRQ

    pop ebx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popa
    add esp, 8
    sti
    iret
