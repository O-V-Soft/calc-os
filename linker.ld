ENTRY(kernel_main)

SECTIONS
{
    . = 0x14200;

    .text : {
        *(.text.entry)
        *(.text*)
    }

    . = ALIGN(4096);
    __rodata_start = .;
    .rodata : {
        *(.rodata*)
        *(.rdata*)
    }
    . = ALIGN(4096);
    __rodata_end = .;

    __data_start = .;
    .data : {
        *(.data*)
    }
    . = ALIGN(4096);
    __data_end = .;

    __bss_start = .;
    .bss : {
        *(.bss*)
        *(COMMON)
    }
    . = ALIGN(4096);
    __bss_end = .;

    .bss.safe : {
        *(.bss.safe)
    }
    . = ALIGN(4096);
    __bss_safe_end = .;

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.reloc)
    }
}