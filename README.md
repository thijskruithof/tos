# tos
Thijs Operating System (a 64bit minimal operating system for x86)

Inspired by:
- https://dev.to/frosnerd/writing-my-own-boot-loader-3mld
- https://wiki.osdev.org/Creating_a_64-bit_kernel
- https://alamot.github.io/os_stage1/

Requirements:
- Linux or Windows (with WSL2)
- Make
- GCC
- Qemu

## Compiling

From a shell in this folder just run:

1. `make -C external/gnu-efi`
2. `make`

After this you should have file name `tos.img` in your `build` directory.

To run this generated image file in Qemu, just type:

`make run`
