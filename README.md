# Sistema Operativo M 8086 (SOM8086)  
*M Operating System 8086*  

This is a project I worked on during 2015/2016. The goal was to build a tiny operating system for the 8086 processor, which was used in early PCs (the x86 32-bit processor family began in 8086 mode). It is written in C and Assembly.  

I made significant progress: I developed a bootstrap program to boot from a floppy disk that loaded the full binary into memory. Once laoded, the operating system runs command line user interface.

The project, however, did not reach the stage where the OS could load other programs or execute commands. Despite this, I find the repository very interesting and well-documented.  

[Legacy documentation (in spanish) here](/docs/LEGACY.md)

---

## Try It Yourself  
The repository includes binaries and disk images, so you can test it directly.  

I created a VirtualBox VM using the `disk/CDARNQ.ISO` image. While it runs, it does not capture keystrokes. The `disk/DISCOM.IMA` file is a raw floppy disk image.  

---

## Instructions  

### How to Compile  
This project was developed on Windows, so the instructions provided are tailored to that environment. You'll likely need [MinGW](https://www.mingw-w64.org/) to use GCC tools like `make`.  

#### `nucleo.bin` (Kernel)  
The `/src` directory contains a Makefile for compiling the kernel. Use the following command to build it:  

```bash
make
```  

It compiles `.asm` and `.c` files into `.obj` files and then links everything into a `.bin` file. Compiler options ensure that only 8086 instructions are used, and the final binary is a pure binary file without a header.  

Required tools:  
- **`wcc`:** [Open Watcom](https://www.openwatcom.org/) C compiler.  
- **`wlink`:** The linker provided with Watcom C/C++.  
- **`nasm`:** [NASM, the Netwide Assembler](https://github.com/netwide-assembler/nasm).  

Ensure these programs are added to the `%PATH%`.  

##### NASM Options  
- `-f` Specifies the output format: OBJ.  

##### OpenWatcom `wcc` Options  
- `-0` Use 8086 instructions.  
- `-d0` Disable debugging information.  
- `-ms` Use "small" memory model (CS = ES = DS).  
- `-s` Disable stack overflow checks.  
- `-wx` Enable maximum warning level.  
- `-zl` Exclude library references in the object file.  
- `-i` Include directory for `.h` files.  

---

#### `arranque` (Bootstrap)  
There is a `crear.bat` (*make*) script for the bootstrap routine. This routine is compiled into exactly 512 bytes and must be written directly to the boot sector of the disk image.  

---

#### Utilities  
Two utilities are provided:  
1. **`insarnq`:** Copies the 512 bytes from the `arranque.bin` file to the specified disk image file.  
2. **`leernbp`:** Displays the boot sector information of a disk image file.  

These utilities are standalone tools and must be compiled directly on your PC's system as target, because they are not part of the OS.  

---

### How to Run  

1. Create a 3.5" 1.44MB floppy disk image. I used [WinImage](http://www.winimage.com) by Gilles Vollant, but the website seems broken. Other similar tools may work.  
2. Overwrite the boot sector with `arranque.bin` using the `insarnq` utility.  
3. Copy `nucleo.bin` into the disk image. **It must be the first file on the disk** because the bootstrap routine blindly loads the first file.  
4. Mount the disk image in a virtual machine.  
5. Enjoy!  