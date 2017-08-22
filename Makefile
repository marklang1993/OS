# Makefile for OS #

# Build Target
K_OBJECTS = kernel_asm.o kernel_c.o proc.o interrupt_asm.o interrupt_c.o syscall_asm.o syscall_c.o ipc.o
LIB_OBJECTS = dbg.o kheap.o memory.o buffer.o file.o print.o printk.o io_port.o
DRV_OBJECTS = i8259a.o i8253.o keyboard.o vga.o tty.o hdd.o hdd_part.o fs.o
OBJECTS = $(K_OBJECTS) $(LIB_OBJECTS) $(DRV_OBJECTS) 

TARGET_BOOT = boot.bin
TARGET = loader.bin kernel.bin
TARGET_IMG = boot.img

UTILITIES = partition

# Tools, Compilers, Flags, etc...
ASM = nasm
ASM_FLAGS = -I include/
ASM_ELF_FLAGS = -f elf -I include/
IMG = dd
IMG_FORMAT = mkdosfs
DISK_IMG_FLAGS = bs=512 count=2880
BOOT_IMG_FLAGS = bs=512 count=1 conv=notrunc
MACHINE = bochs
LINKER = ld
GCC = gcc
GCC_FLAGS = -m32 -c -D _OS_DBG_ -fno-zero-initialized-in-bss -fno-builtin -fno-stack-protector -Werror -I include/
GPP = g++
GPP_FLAGS = -std=c++11 -I include/ -I utility/include/
# NOTE: 0x50000(Defined by KernelBaseOffset) + 0x400 (ELF header and other headers)
# LINKER_FLAGS = -s -m elf_i386 -Ttext 0x50400
LINKER_FLAGS = -m elf_i386 -Ttext 0x50400

# Phony Targets
.PHONY : all clean

# Start Position
all : $(OBJECTS) $(TARGET_BOOT) $(TARGET) $(TARGET_IMG)
	   sudo mount -o loop $(TARGET_IMG) /mnt/floppy
	   sudo cp $(TARGET) /mnt/floppy/ 
	   sudo umount /mnt/floppy

utility : $(UTILITIES)

run : 
	   $(MACHINE) -f bochsrc
clean : 
	   rm -f $(TARGET_BOOT)
	   rm -f $(TARGET)
	   rm -f $(OBJECTS)
	   rm -f $(TARGET_IMG)
	   rm -f $(UTILITIES)

kernel_asm.o : 	kernel/kernel.asm
		$(ASM) $(ASM_ELF_FLAGS) -o $@ $<

kernel_c.o : 	kernel/kernel.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

proc.o : 	kernel/proc.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

interrupt_asm.o : kernel/interrupt.asm
		$(ASM) $(ASM_ELF_FLAGS) -o $@ $<

interrupt_c.o : kernel/interrupt.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

syscall_asm.o : kernel/syscall.asm
		$(ASM) $(ASM_ELF_FLAGS) -o $@ $<

syscall_c.o :   kernel/syscall.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

ipc.o :   	kernel/ipc.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

dbg.o:		lib/dbg.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

kheap.o :	lib/kheap.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

memory.o : 	lib/memory.asm
		$(ASM) $(ASM_ELF_FLAGS) -o $@ $<

buffer.o :	lib/buffer.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

file.o :	lib/file.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

print.o : 	lib/print.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

printk.o : 	lib/printk.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

io_port.o : 	lib/io_port.asm
		$(ASM) $(ASM_ELF_FLAGS) -o $@ $<

i8259a.o :	kernel/drivers/i8259a.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

i8253.o :	kernel/drivers/i8253.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

keyboard.o :	kernel/drivers/keyboard.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

vga.o :		kernel/drivers/vga.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

tty.o :		kernel/drivers/tty.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

hdd.o :		kernel/drivers/hdd.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

hdd_part.o :	kernel/drivers/hdd_part.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

fs.o :		kernel/drivers/fs/fs.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

boot.bin : 	boot/boot.asm
		$(ASM) $(ASM_FLAGS) -o $@ $<

loader.bin : 	boot/loader.asm
		$(ASM) $(ASM_FLAGS) -o $@ $<

kernel.bin :	$(OBJECTS)
		$(LINKER) $(LINKER_FLAGS) -o $@ $^

boot.img : 	$(TARGET_BOOT)
		$(IMG) if=/dev/zero of=$(TARGET_IMG) $(DISK_IMG_FLAGS)
		$(IMG_FORMAT) -F 12 -v $(TARGET_IMG) 
		$(IMG) if=$< of=$(TARGET_IMG) $(BOOT_IMG_FLAGS)

partition:	utility/fs/partition.cpp
		$(GPP) $(GPP_FLAGS) -o $@ $<

	 
