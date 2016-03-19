ASM = nasm
ASMFLAGS = -I Boot/

TARGET = boot.bin

all : $(TARGET)

clean : 
	rm -f $(TARGET)

bochs :
	dd if=boot.bin of=boot.img bs=512 count=1 conv=notrunc
	bochs

boot.bin : Boot/boot.asm
	$(ASM) $(ASMFLAGS) -o $@ $<
