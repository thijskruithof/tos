BUILDDIR := bin
IMAGEFILE := $(BUILDDIR)/tos.img
OVMFDIR = external/OVMF

all: buildbootloader buildkernel buildimg

buildbootloader: 
	make -C src/bootloader

buildkernel:
	make -C src/kernel

buildimg:
	dd if=/dev/zero of=$(IMAGEFILE) bs=512 count=2880
	mformat -i $(IMAGEFILE) -f 1440 ::
	mmd -i $(IMAGEFILE) ::/EFI
	mmd -i $(IMAGEFILE) ::/EFI/BOOT
	mcopy -i $(IMAGEFILE) src/bootloader/main.efi ::/EFI/BOOT/BOOTX64.EFI
	mcopy -i $(IMAGEFILE) $(BUILDDIR)/kernel.elf ::

run:
# Qemu cmdline notes:
# -net None => disable ipxe network booting 
# -s => start a gdb debugger on port 1234
# -S => wait for gdb connection at startup
	qemu-system-x86_64 -drive file=$(IMAGEFILE) -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="$(OVMFDIR)/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="$(OVMFDIR)/OVMF_VARS-pure-efi.fd" -net none -s -S

clean:
	rm -f $(IMAGEFILE)
	make -C src/bootloader clean 
	make -C src/kernel clean 