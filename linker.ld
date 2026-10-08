/* ============================================================================
 * Linker Script для GraphOS Kernel
 * Описывает как компоновать ядро в памяти
 * ============================================================================ */

OUTPUT_FORMAT(binary)
ENTRY(_start)

SECTIONS
{
    /* Ядро загружается по адресу 0x1000 (совпадает с KERNEL_OFFSET в boot.asm) */
    . = 0x1000;

    .text ALIGN(4K) : {
        *(.text)
    }

    .rodata ALIGN(4K) : {
        *(.rodata*)
    }

    .data ALIGN(4K) : {
        *(.data)
    }

    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }
}
