#
#  Copyright (C) 1999-2001 Hewlett-Packard Co.
#	Contributed by David Mosberger <davidm@hpl.hp.com>
#	Contributed by Stephane Eranian <eranian@hpl.hp.com>
#
#    All rights reserved.
#
#    Redistribution and use in source and binary forms, with or without
#    modification, are permitted provided that the following conditions
#    are met:
#
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following
#      disclaimer in the documentation and/or other materials
#      provided with the distribution.
#    * Neither the name of Hewlett-Packard Co. nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
#
#    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
#    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
#    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
#    BE LIABLE FOR ANYDIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
#    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
#    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
#    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
#    SUCH DAMAGE.
#

SRCDIR = .

VPATH = $(SRCDIR)

include $(SRCDIR)/../../external/gnu-efi/Make.defaults

TOPDIR = $(SRCDIR)/../../external/gnu-efi

CDIR=$(TOPDIR)/..
LINUX_HEADERS	= /usr/src/sys/build
CPPFLAGS	+= -D__KERNEL__ -I$(LINUX_HEADERS)/include
CRTOBJS		= $(SRCDIR)/../../external/gnu-efi/$(ARCH)/gnuefi/crt0-efi-$(ARCH).o

LDSCRIPT	= $(TOPDIR)/gnuefi/elf_$(ARCH)_efi.lds
ifneq (,$(findstring FreeBSD,$(OS)))
LDSCRIPT	= $(TOPDIR)/gnuefi/elf_$(ARCH)_fbsd_efi.lds
endif

LDFLAGS		+= -shared -Bsymbolic -L$(SRCDIR)/../../external/gnu-efi/$(ARCH)/lib -L$(SRCDIR)/../../external/gnu-efi/$(ARCH)/gnuefi $(CRTOBJS)

LOADLIBES	+= -lefi -lgnuefi
LOADLIBES	+= $(LIBGCC)
LOADLIBES	+= -T $(LDSCRIPT)

TARGET_APPS = main.efi
TARGET_BSDRIVERS =
TARGET_RTDRIVERS =

ifneq ($(HAVE_EFI_OBJCOPY),)

FORMAT		:= --target efi-app-$(ARCH)
$(TARGET_BSDRIVERS): FORMAT=--target efi-bsdrv-$(ARCH)
$(TARGET_RTDRIVERS): FORMAT=--target efi-rtdrv-$(ARCH)

else

SUBSYSTEM	:= 0xa
$(TARGET_BSDRIVERS): SUBSYSTEM = 0xb
$(TARGET_RTDRIVERS): SUBSYSTEM = 0xc

FORMAT		:= -O binary
LDFLAGS		+= --defsym=EFI_SUBSYSTEM=$(SUBSYSTEM)

endif

TARGETS = $(TARGET_APPS) $(TARGET_BSDRIVERS) $(TARGET_RTDRIVERS)

CFLAGS += -Wno-error=unused-parameter -Wno-error=unused-variable

all:	$(TARGETS)

clean:
	rm -f $(TARGETS) *~ *.o *.so

.PHONY: install

include $(SRCDIR)/../../external/gnu-efi/Make.rules


# CC = gcc
# GNUEFI = ../../external/gnu-efi

# CFLAGS = -ffreestanding -fshort-wchar -I$(GNUEFI)/inc -I$(GNUEFI)/inc/x86_64 -I$(GNUEFI)/inc/protocol -g
# LINK_CFLAGS = -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main

# SRCDIR := .
# OBJDIR := ../../obj
# BUILDDIR := ../../bin
# BOOTEFI := $(BUILDDIR)/bootx64.efi

# rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# SRC = $(call rwildcard,$(SRCDIR),*.c)          
# OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
# DIRS = $(wildcard $(SRCDIR)/*)

# all: $(OBJS) link

# $(OBJDIR)/%.o: $(SRCDIR)/%.c
# 	@ echo !==== COMPILING BOOTLOADER $^
# 	@ mkdir -p $(@D)
# 	$(CC) $(CFLAGS) -c $^ -o $@
	
# link:
# 	@ echo !==== LINKING BOOTLOADER
# 	$(CC) $(LINK_CFLAGS) -o $(BUILDDIR)/kernel.elf $(KERNEL_OBJS)


# # # $@ = target file
# # # $< = first dependency
# # # $^ = all dependencies

# # GCC := gcc 
# # CC = gcc
# # LD = ld

# # all: bin\disk\EFI\BOOT\BOOTX64.EFI

# # # -ffreestanding -fshort-wchar -g

# # bin/main.o: src/main.c 
# # 	$(GCC) -ffreestanding -Iexternal/gnu-efi/inc -Iexternal/gnu-efi/inc/x86_64 -Iexternal/gnu-efi/inc/protocol -fshort-wchar -g -c -o $@ $<

# # bin/libgnuefi_data.o: src/libgnuefi_data.c 
# # 	$(GCC) -ffreestanding -Iexternal/gnu-efi/inc -Iexternal/gnu-efi/inc/x86_64 -Iexternal/gnu-efi/inc/protocol -fshort-wchar -g -c -o $@ $<

# # bin/disk/EFI/BOOT/BOOTX64.EFI: bin/main.o bin/libgnuefi_data.o 
# # 	rm -r -f bin/disk/
# # 	mkdir -p bin/disk/EFI/BOOT
# # 	$(GCC) -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -o $@ $^

# # run: bin\disk\EFI\BOOT\BOOTX64.EFI
# # 	qemu-system-x86_64 --bios external\OVMF\OVMF-pure-efi.fd -drive file=fat:floppy:rw:bin\disk,format=raw
# # #	bochsdbg -f bin\bochsrc.bxrc -q


# # clean:
# # 	$(RM) bin\*.bin bin\*.o bin\*.dis