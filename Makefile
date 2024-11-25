# $@ = target file
# $< = first dependency
# $^ = all dependencies

GCC := x86_64-w64-mingw32-gcc

all: bin\BOOTX64.EFI

bin\main.o: src\main.c 
	$(GCC) -ffreestanding -Iexternal/gnu-efi/inc -Iexternal/gnu-efi/inc/x86_64 -Iexternal/gnu-efi/inc/protocol -c -o $@ $<

bin\libgnuefi_data.o: src\libgnuefi_data.c 
	$(GCC) -ffreestanding -Iexternal/gnu-efi/inc -Iexternal/gnu-efi/inc/x86_64 -Iexternal/gnu-efi/inc/protocol -c -o $@ $<

bin\BOOTX64.EFI: bin\main.o bin\libgnuefi_data.o 
	$(GCC) -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -o $@ $^

# bin\tos.bin: src\tos.asm src\boot\bootstage0.asm src\boot\bootstage1.asm src\boot\disk.asm src\boot\print.asm
# 	nasm -isrc\ src\tos.asm -f bin -o $@ -l bin\tos.lst
# 	ndisasm -b 16 $@ > bin\tos.disasm

# bin\tos.bin: bin\mbr.bin #bin\kernel.bin
# copy /b bin\mbr.bin+bin\kernel.bin $@
# copy /b bin\mbr.bin $@

run: bin\BOOTX64.EFI
	qemu-system-x86_64 --bios external\OVMF\OVMF-pure -drive file=fat:floppy:rw:bin,format=raw
#	bochsdbg -f bin\bochsrc.bxrc -q


clean:
	$(RM) bin\*.bin bin\*.o bin\*.dis