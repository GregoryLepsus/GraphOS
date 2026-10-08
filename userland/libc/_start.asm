; ============================================================================
; User Program Startup Code
; Точка входа для всех пользовательских программ
; ============================================================================

global _start

extern main
extern exit

section .text

_start:
    ; Стек уже настроен ядром
    ; ESP указывает на argc
    ; ESP+4 указывает на argv

    ; Получить argc и argv из стека
    pop eax             ; argc
    mov ebx, esp        ; argv (указатель на массив)

    ; Вызвать main(argc, argv)
    push ebx            ; argv
    push eax            ; argc
    call main
    add esp, 8          ; Очистить стек

    ; main вернул значение в eax
    ; Вызвать exit(return_value)
    push eax
    call exit

    ; Если exit не сработал, зависнуть
    jmp $
