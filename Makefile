# $@ = target file
# $< = first dependency
# $^ = all dependencies

# First rule is the one executed when no parameters are fed to the Makefile
all: run

bin\kernel.bin: bin\kernel-entry.o bin\kernel.o
# ld -m i386pe -o $@ -Ttext 0x1000 $^ --oformat binary
	ld -Map bin\kernel.map -m i386pe -T NUL -o bin\kernel.tmp -Ttext 0x1000 $^
	objcopy -O binary -j .text bin\kernel.tmp $@
	

bin\kernel-entry.o: src\kernel-entry.asm
	nasm -isrc\ $< -f win32 -o $@

bin\kernel.o: src\kernel.c
	gcc -m32 -ffreestanding -c $< -o $@

bin\mbr.bin: src\mbr.asm
	nasm -isrc\ $^ -f bin -o $@

bin\tos.bin: bin\mbr.bin bin\kernel.bin
	copy /b bin\mbr.bin+bin\kernel.bin $@

run: bin\tos.bin
	qemu-system-i386 -fda $<

clean:
	$(RM) bin\*.bin bin\*.o bin\*.dis