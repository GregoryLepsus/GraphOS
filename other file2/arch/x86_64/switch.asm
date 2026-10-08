; ============================================================================
; Context Switching (x86 Assembly)
; Переключение контекста между процессами
; ============================================================================

[BITS 32]
section .text

; Экспортируемая функция
global switch_context

; void switch_context(cpu_context_t* old_ctx, cpu_context_t* new_ctx)
switch_context:
    ; Получить аргументы
    mov eax, [esp + 4]  ; old_ctx
    mov edx, [esp + 8]  ; new_ctx

    ; Проверить, есть ли old_ctx
    test eax, eax
    jz .load_new        ; Если нет old_ctx, сразу загружаем new

    ; Сохранить контекст old_ctx
    mov [eax + 0],  ebx     ; ebx (используем ebx вместо eax, т.к. eax занят)
    mov [eax + 4],  ebx     ; ebx
    mov [eax + 8],  ecx     ; ecx
    mov [eax + 12], edx     ; edx (сохраняем сейчас, пока edx не используется)
    mov [eax + 16], esi     ; esi
    mov [eax + 20], edi     ; edi
    mov [eax + 24], ebp     ; ebp

    ; Сохранить esp (до вызова функции)
    lea ebx, [esp + 4]      ; esp до call (пропускаем return address)
    mov [eax + 28], ebx     ; esp

    ; Сохранить eip (return address)
    mov ebx, [esp]          ; return address
    mov [eax + 32], ebx     ; eip

    ; Сохранить eflags
    pushfd
    pop ebx
    mov [eax + 36], ebx     ; eflags

    ; Сохранить CR3
    mov ebx, cr3
    mov [eax + 40], ebx     ; cr3

.load_new:
    ; Загрузить новый CR3 (page directory)
    mov ebx, [edx + 40]
    mov cr3, ebx

    ; Загрузить регистры из new_ctx
    mov ebx, [edx + 4]      ; ebx
    mov ecx, [edx + 8]      ; ecx
    mov esi, [edx + 16]     ; esi
    mov edi, [edx + 20]     ; edi
    mov ebp, [edx + 24]     ; ebp
    mov esp, [edx + 28]     ; esp

    ; Загрузить eflags
    mov eax, [edx + 36]
    push eax
    popfd

    ; Восстановить eax, edx последними
    mov eax, [edx + 0]      ; eax
    push dword [edx + 12]   ; edx на стек

    ; Запушить новый eip на стек для ret
    push dword [edx + 32]   ; eip

    ; Восстановить edx
    pop edx                 ; edx

    ; "Вернуться" к новому eip
    ret
