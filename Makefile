# $@ = target file
# $< = first dependency
# $^ = all dependencies

GCC := x86_64-w64-mingw32-gcc

all: bin\disk\EFI\BOOT\BOOTX64.EFI

bin\main.o: src\main.c 
	$(GCC) -ffreestanding -Iexternal/gnu-efi/inc -Iexternal/gnu-efi/inc/x86_64 -Iexternal/gnu-efi/inc/protocol -c -o $@ $<

bin\libgnuefi_data.o: src\libgnuefi_data.c 
	$(GCC) -ffreestanding -Iexternal/gnu-efi/inc -Iexternal/gnu-efi/inc/x86_64 -Iexternal/gnu-efi/inc/protocol -c -o $@ $<

bin\disk\EFI\BOOT\BOOTX64.EFI: bin\main.o bin\libgnuefi_data.o 
	del /S /Q /F bin\disk
	md bin\disk\EFI\BOOT
	$(GCC) -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -o $@ $^

run: bin\disk\EFI\BOOT\BOOTX64.EFI
	qemu-system-x86_64 --bios external\OVMF\OVMF-pure-efi.fd -drive file=fat:floppy:rw:bin\disk,format=raw
#	bochsdbg -f bin\bochsrc.bxrc -q


clean:
	$(RM) bin\*.bin bin\*.o bin\*.dis