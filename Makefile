# Makefile for OS #

# Build Target
OBJECT = kernel.o
TARGET = boot.bin loader.bin kernel.bin
TARGET_IMG = boot.img

# Tools, Compilers, Flags, etc...
ASM = nasm
ASM_FLAGS = -I include/
ASM_ELF_FLAGS = -f elf
IMG = dd
IMG_FORMAT = mkdosfs
DISK_IMG_FLAGS = bs=512 count=2880
BOOT_IMG_FLAGS = bs=512 count=1 conv=notrunc
MACHINE = bochs
LINKER = ld
# NOTE: 0x50000(Defined by KernelBaseOffset) + 0x400 (ELF header and other headers)
LINKER_FLAGS = -m elf_i386 -s -Ttext 0x50400

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

kernel.o : kernel/kernel.asm
	   $(ASM) $(ASM_ELF_FLAGS) -o $@ $<

boot.bin : asm/boot.asm include/boot.inc
	   $(ASM) $(ASM_FLAGS) -o $@ $<

loader.bin : asm/loader.asm include/boot.inc
	     $(ASM) $(ASM_FLAGS) -o $@ $<

kernel.bin : kernel.o
	     $(LINKER) $(LINKER_FLAGS) -o $@ $< 

boot.img : boot.bin
	   $(IMG) if=/dev/zero of=$(TARGET_IMG) $(DISK_IMG_FLAGS)
	   $(IMG_FORMAT) -F 12 -v $(TARGET_IMG) 
	   $(IMG) if=$< of=$(TARGET_IMG) $(BOOT_IMG_FLAGS)
	 
