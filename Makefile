# BlockOS Makefile

CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin \
         -fno-stack-protector -nostartfiles \
         -nodefaultlibs -Wall -Wextra -c

ASFLAGS = -f elf32

SRCDIR = .
OBJDIR = build

SOURCES = $(shell find . -name "*.c" ! -path "./install/*")
OBJECTS = $(SOURCES:.c=.o)

all: blockos.bin

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

%.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

blockos.bin: $(OBJECTS)
	$(LD) -m elf_i386 -T linker.ld -o $@ $^

iso: blockos.bin
	mkdir -p iso/boot/grub
	cp blockos.bin iso/boot/
	grub-mkrescue -o blockos.iso iso/

clean:
	find . -name "*.o" -delete
	rm -f blockos.bin blockos.iso

.PHONY: all iso clean
