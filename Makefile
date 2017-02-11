# Makefile for OS #

# Build Target
TARGET = boot.bin
TARGET_IMG = boot.img

# Tools, Compilers, Flags, etc...
ASM = nasm
ASM_FLAGS = -I include/
IMG = dd
DISK_IMG_FLAGS = bs=512 count=2880
BOOT_IMG_FLAGS = bs=512 count=1 conv=notrunc
MACHINE = bochs

# Phony Targets
.PHONY : all clean

# Start Position
all : $(TARGET) $(TARGET_IMG)

run : 
	   $(MACHINE) -f bochsrc
clean : 
	   rm -f $(TARGET)
	   rm -f $(TARGET_IMG)

boot.bin : boot/boot.asm include/boot.inc
	   $(ASM) $(ASM_FLAGS) -o $@ $<
boot.img : boot.bin
	   $(IMG) if=/dev/zero of=$(TARGET_IMG) $(DISK_IMG_FLAGS)
	   $(IMG) if=$< of=$(TARGET_IMG) $(BOOT_IMG_FLAGS)

