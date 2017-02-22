# Makefile for OS #

# Build Target
OBJECT = kernel.o kernel_init.o memcpy.o print_string.o io_port.o interrupt.o
TARGET = boot.bin loader.bin kernel.bin
TARGET_IMG = boot.img

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
GCC_FLAGS = -m32 -c -fno-builtin -I include/
# NOTE: 0x50000(Defined by KernelBaseOffset) + 0x400 (ELF header and other headers)
LINKER_FLAGS = -s -m elf_i386 -Ttext 0x50400

# Phony Targets
.PHONY : all clean

# Start Position
all : $(OBJECT) $(TARGET) $(TARGET_IMG)
	   sudo mount -o loop $(TARGET_IMG) /mnt/floppy
	   sudo cp loader.bin /mnt/floppy/
	   sudo cp kernel.bin /mnt/floppy/ 
	   sudo umount /mnt/floppy

run : 
	   $(MACHINE) -f bochsrc
clean : 
	   rm -f $(TARGET)
	   rm -f $(OBJECT)
	   rm -f $(TARGET_IMG)

kernel_init.o : kernel/kernel_init.c
		$(GCC) $(GCC_FLAGS) -o $@ $<

kernel.o : kernel/kernel.asm
	   $(ASM) $(ASM_ELF_FLAGS) -o $@ $<

memcpy.o : lib/memcpy.asm
	   $(ASM) $(ASM_ELF_FLAGS) -o $@ $<

print_string.o :  lib/print_string.asm
	   	  $(ASM) $(ASM_ELF_FLAGS) -o $@ $<

io_port.o : lib/io_port.asm
	    $(ASM) $(ASM_ELF_FLAGS) -o $@ $<

interrupt.o : lib/interrupt.asm
	    $(ASM) $(ASM_ELF_FLAGS) -o $@ $<

boot.bin : asm/boot.asm
	   $(ASM) $(ASM_FLAGS) -o $@ $<

loader.bin : asm/loader.asm
	     $(ASM) $(ASM_FLAGS) -o $@ $<

kernel.bin :
	     $(LINKER) $(LINKER_FLAGS) -o $@ $(OBJECT) 

boot.img : boot.bin
	   $(IMG) if=/dev/zero of=$(TARGET_IMG) $(DISK_IMG_FLAGS)
	   $(IMG_FORMAT) -F 12 -v $(TARGET_IMG) 
	   $(IMG) if=$< of=$(TARGET_IMG) $(BOOT_IMG_FLAGS)
	 
