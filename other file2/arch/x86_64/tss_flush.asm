; ============================================================================
; TSS Flush - Load Task Register
; ============================================================================

global tss_flush

tss_flush:
    mov ax, 0x2B       ; TSS segment selector (GDT entry 5, RPL=3)
    ltr ax             ; Load Task Register
    ret
