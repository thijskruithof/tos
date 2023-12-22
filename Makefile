# $@ = target file
# $< = first dependency
# $^ = all dependencies

# First rule is the one executed when no parameters are fed to the Makefile
all: bin\tos.bin

# bin\kernel.bin: bin\kernel-entry.o bin\kernel.o
# # ld -m i386pe -o $@ -Ttext 0x1000 $^ --oformat binary
# 	ld -Map bin\kernel.map -m i386pe -T NUL -o bin\kernel.tmp -Ttext 0x1000 $^
# 	objcopy -O binary -j .text bin\kernel.tmp $@
	

# bin\kernel-entry.o: src\kernel-entry.asm
# 	nasm -isrc\ $< -f win32 -o $@

# bin\kernel.o: src\kernel.c
# 	gcc -m64 -nostdinc -nostdlib -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -c $< -o $@

bin\tos.bin: src\tos.asm src\boot\bootstage0.asm src\boot\bootstage1.asm src\boot\disk.asm src\boot\print.asm
	nasm -isrc\ src\tos.asm -f bin -o $@ -l bin\tos.lst
	ndisasm -b 16 $@ > bin\tos.disasm

# bin\tos.bin: bin\mbr.bin #bin\kernel.bin
# copy /b bin\mbr.bin+bin\kernel.bin $@
# copy /b bin\mbr.bin $@

run: bin\tos.bin
# -s -S is for "wait for gdb debugger to attach on tcp port 1234"
	qemu-system-i386 -fda $< 

clean:
	$(RM) bin\*.bin bin\*.o bin\*.dis