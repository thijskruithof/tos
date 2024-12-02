# # $@ = target file
# # $< = first dependency
# # $^ = all dependencies

LDS = kernel.ld
CC = gcc
LD = ld

CFLAGS = -ffreestanding -fshort-wchar
LDFLAGS = -T $(LDS) -static -Bsymbolic -nostdlib

SRCDIR := .
OBJDIR := ../../obj/kernel
BUILDDIR := ../../bin

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC = $(call rwildcard,$(SRCDIR),*.c)          
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
DIRS = $(wildcard $(SRCDIR)/*)


kernel: $(OBJS) linkkernel


$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ echo !==== COMPILING KERNEL $^
	@ mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $^ -o $@
	
linkkernel:
	@ echo !==== LINKING KERNEL
	$(LD) $(LDFLAGS) -o $(BUILDDIR)/kernel.elf $(OBJS)

clean:
	rm -f $(OBJDIR)/*.o 
	rm -f $(BUILDDIR)/kernel.elf