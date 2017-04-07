; 8259A Ports
PORT_8259A_MAIN_0               equ             020h
PORT_8259A_MAIN_1               equ             021h
PORT_8259A_SLAVE_0              equ             0a0h
PORT_8259A_SLAVE_1              equ             0a1h


; ICW1 -> PORT0, ICW2~4 -> PORT1
DATA_8259A_ICW1                 equ             11h
DATA_8259A_MAIN_ICW2            equ             20h    ; IR0 <-> 32
DATA_8259A_SLAVE_ICW2           equ             28h    ; IR8 <-> 40
DATA_8259A_MAIN_ICW3            equ             04h    ; Main IR2 (3rd bit) -> Slave 8259A
DATA_8259A_SLAVE_ICW3           equ             02h    ; Slave 8259A -> Main IR2
DATA_8259A_ICW4                 equ             01h    ; x86 mode, auto EOI

; OCW1 -> PORT1, OCW2(EOI) -> PORT0
DATA_8259A_MAIN_OCW1            equ             0ffh   ; Disable all master inerrrupts
DATA_8259A_SLAVE_OCW1           equ             0ffh   ; Disable all slave interrupts
DATA_8259A_OCW2                 equ             020h   ; Send EOI to notify 8259A interrupt processed

; 8259A interrupt index
INDEX_8259A_CLOCK		equ		0
INDEX_8259A_KEYBOARD		equ		1
INDEX_8259A_SLAVE		equ		2
INDEX_8259A_HDD			equ		14

COUNT_INT_8259A_MASTER		equ		8
COUNT_INT_8259A_SLAVE		equ		8
COUNT_INT_8259A			equ		16

