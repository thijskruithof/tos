GNUEFI = ../external/gnu-efi
OVMFDIR = ../external/OVMF
LDS = kernel.ld
CC = gcc
LD = ld

CFLAGS = -ffreestanding -fshort-wchar
LDFLAGS = -T $(LDS) -static -Bsymbolic -nostdlib

KERNEL_SRCDIR := src/kernel
KERNEL_OBJDIR := obj/kernel
BUILDDIR = bin
BOOTEFI := $(GNUEFI)/x86_64/bootloader/main.efi

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

KERNEL_SRC = $(call rwildcard,$(KERNEL_SRCDIR),*.c)          
KERNEL_OBJS = $(patsubst $(KERNEL_SRCDIR)/%.c, $(KERNEL_OBJDIR)/%.o, $(KERNEL_SRC))
KERNEL_DIRS = $(wildcard $(KERNEL_SRCDIR)/*)

kernel: $(KERNEL_OBJS) linkkernel

$(KERNEL_OBJDIR)/%.o: $(KERNEL_SRCDIR)/%.c
	@ echo !==== COMPILING KERNEL $^
	@ mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $^ -o $@
	
linkkernel:
	@ echo !==== LINKING KERNEL
	$(LD) $(LDFLAGS) -o $(BUILDDIR)/kernel.elf $(KERNEL_OBJS)

setup:
	@mkdir $(BUILDDIR)
	@mkdir $(KERNEL_SRCDIR)
	@mkdir $(KERNEL_OBJDIR)

buildimg:
	dd if=/dev/zero of=$(BUILDDIR)/tos.img bs=512 count=2880
	mformat -i $(BUILDDIR)/tos.img -f 1440 ::
	mmd -i $(BUILDDIR)/tos.img ::/EFI
	mmd -i $(BUILDDIR)/tos.img ::/EFI/BOOT
	# mcopy -i $(BUILDDIR)/tos.img $(BOOTEFI) ::/EFI/BOOT
	mcopy -i $(BUILDDIR)/tos.img $(BUILDDIR)/kernel.elf ::

run:
	qemu-system-x86_64 -drive file=$(BUILDDIR)/tos.img -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="$(OVMFDIR)/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="$(OVMFDIR)/OVMF_VARS-pure-efi.fd" -net none

# # $@ = target file
# # $< = first dependency
# # $^ = all dependencies

# GCC := gcc 
# CC = gcc
# LD = ld

# all: bin\disk\EFI\BOOT\BOOTX64.EFI

# # -ffreestanding -fshort-wchar -g

# bin/main.o: src/main.c 
# 	$(GCC) -ffreestanding -Iexternal/gnu-efi/inc -Iexternal/gnu-efi/inc/x86_64 -Iexternal/gnu-efi/inc/protocol -fshort-wchar -g -c -o $@ $<

# bin/libgnuefi_data.o: src/libgnuefi_data.c 
# 	$(GCC) -ffreestanding -Iexternal/gnu-efi/inc -Iexternal/gnu-efi/inc/x86_64 -Iexternal/gnu-efi/inc/protocol -fshort-wchar -g -c -o $@ $<

# bin/disk/EFI/BOOT/BOOTX64.EFI: bin/main.o bin/libgnuefi_data.o 
# 	rm -r -f bin/disk/
# 	mkdir -p bin/disk/EFI/BOOT
# 	$(GCC) -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -o $@ $^

# run: bin\disk\EFI\BOOT\BOOTX64.EFI
# 	qemu-system-x86_64 --bios external\OVMF\OVMF-pure-efi.fd -drive file=fat:floppy:rw:bin\disk,format=raw
# #	bochsdbg -f bin\bochsrc.bxrc -q


# clean:
# 	$(RM) bin\*.bin bin\*.o bin\*.dis