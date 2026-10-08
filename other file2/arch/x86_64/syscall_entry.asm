; ============================================================================
; System Call Entry Point (x86 Assembly)
; Точка входа для системных вызовов через int 0x80
; ============================================================================

[BITS 32]
section .text

; Экспортируемая функция
global syscall_entry
extern syscall_dispatcher

; System call entry point (called from int 0x80)
syscall_entry:
    ; Сохранить все регистры
    push ebp
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax

    ; Передать параметры в syscall_dispatcher
    ; eax = syscall number
    ; ebx = arg1, ecx = arg2, edx = arg3, esi = arg4, edi = arg5
    push edi        ; arg5
    push esi        ; arg4
    push edx        ; arg3
    push ecx        ; arg2
    push ebx        ; arg1
    push eax        ; syscall_num

    ; Вызвать C обработчик
    call syscall_dispatcher

    ; Очистить стек (6 аргументов × 4 байта)
    add esp, 24

    ; Результат в eax уже установлен syscall_dispatcher
    ; Сохранить результат
    mov [esp], eax  ; Заменить сохранённый eax результатом

    ; Восстановить регистры (кроме eax - там результат)
    pop eax         ; Результат системного вызова
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp

    ; Возврат из прерывания
    iret
