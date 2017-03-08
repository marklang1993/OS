%include "boot/pm.inc"

; Kernel GDT Selector
KERNEL_GDT_FLAT_C_Selector              equ             1 << 3
KERNEL_GDT_FLAT_DRW_Selector            equ             2 << 3
KERNEL_GDT_VIDEO_Selector               equ             (3 << 3) + SEL_RPL_3
KERNEL_GDT_FLAT_TSS_Selector            equ             4 << 3
KERNEL_GDT_FLAT_LDT_0_Selector          equ             5 << 3

; TSS
TSS_ESP_0_OFFSET			equ		4
