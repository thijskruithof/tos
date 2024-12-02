@pushd %~dp0

qemu-system-x86_64 -drive file=bin/tos.img -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="external/OVMF/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="external/OVMF/OVMF_VARS-pure-efi.fd" -net none -s -S
pause