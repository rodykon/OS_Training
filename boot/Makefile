
mbr: mbr.asm
	nasm -f bin mbr.asm

boot: mbr boot_seg
	cat mbr boot_seg > boot

all: boot
