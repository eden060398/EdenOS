ARCH:=i686
ASM_FORMAT:=elf32
BITS:=32

CC:=C:/cygwin64/home/Eden0/opt/cross/bin/$(ARCH)-elf-gcc
CFLAGS:=-ffreestanding -Iinclude -Wall
LD:=C:/cygwin64/home/Eden0/opt/cross/bin/$(ARCH)-elf-ld
ASSMBLER:=C:/Users/Eden0/AppData/Local/nasm/nasm.exe

KERNEL_OBJ_FILES:=kernel/kernel_asm$(BITS).o kernel/kernel_c.o
KERNEL:=kernel/kernel$(BITS).bin
BOOTLOADER_ASM:=boot/bootloader$(BITS).asm
BOOTLOADER_SRC_FILES:=$(BOOTLOADER_ASM) boot/memory.asm
BOOTLOADER:=boot/bootloader$(BITS)

INCLUDE_OBJ_FILES:=kernel/console.o kernel/string.o kernel/idt$(BITS).o kernel/interrupts$(BITS).o kernel/keyboard.o kernel/time.o kernel/cmd.o kernel/memory.o kernel/dmemory.o kernel/paging.o kernel/processing.o kernel/pci.o kernel/usb.o kernel/bbb.o kernel/scsi.o kernel/edenfs.o

all: kernel$(BITS).img

kernel$(BITS).img: $(BOOTLOADER) $(KERNEL)
	cat $(BOOTLOADER) $(KERNEL) > kernel$(BITS).img

boot/bootloader$(BITS): $(BOOTLOADER_SRC_FILES)
	$(ASSMBLER) -f bin -o $(BOOTLOADER) $(BOOTLOADER_ASM)

kernel/kernel$(BITS).bin: $(KERNEL_OBJ_FILES) $(INCLUDE_OBJ_FILES)
	$(LD) -T kernel/linker.ld -o $(KERNEL) $(KERNEL_OBJ_FILES) $(INCLUDE_OBJ_FILES)

kernel/kernel_asm$(BITS).o: kernel/kernel_asm$(BITS).asm kernel/isr$(BITS).asm
	$(ASSMBLER) -f $(ASM_FORMAT) -o kernel/kernel_asm$(BITS).o kernel/kernel_asm$(BITS).asm
kernel/kernel_c.o: kernel/kernel_c.c
	$(CC) $(CFLAGS) -c -o kernel/kernel_c.o kernel/kernel_c.c

kernel/%.o: include_src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f kernel$(BITS).img $(KERNEL) $(BOOTLOADER) $(KERNEL_OBJ_FILES) $(INCLUDE_OBJ_FILES)
