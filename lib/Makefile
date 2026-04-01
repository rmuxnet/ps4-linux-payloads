.PHONY: all clean regen-syscalls

ASM ?= $(shell command -v yasm 2>/dev/null || command -v nasm 2>/dev/null)

all: lib.a

clean:
	rm -f lib.a *.o

regen-syscalls: syscalls.py
	python3 syscalls.py > syscalls.asm

dl.o: dl.c
	gcc -c -isystem ../freebsd-headers -nostdinc -fno-stack-protector dl.c -o dl.o -fPIE -ffreestanding

crt.o: crt.asm
	@if [ -z "$(ASM)" ]; then echo "error: need yasm or nasm to assemble crt.asm" >&2; exit 1; fi
	$(ASM) -f elf64 crt.asm

syscalls.o: syscalls.asm
	@if [ -z "$(ASM)" ]; then echo "error: need yasm or nasm to assemble syscalls.asm" >&2; exit 1; fi
	$(ASM) -f elf64 syscalls.asm

lib.a: crt.o dl.o syscalls.o
	ld -r crt.o dl.o syscalls.o -o lib.a
