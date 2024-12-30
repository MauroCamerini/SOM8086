# Sistema Operativo M 8086 (SOM8086)  
*M Operating System 8086*  

This is a project I worked on during 2015/2016. The goal was to build a tiny operating system for the 8086 processor, which was used in early PCs (the x86 32-bit processor family began in 8086 mode). It is written in C and Assembly.  

I made significant progress: I developed a bootstrap program to boot from a floppy disk that loaded the full binary into memory. Once laoded, the operating system runs command line user interface.

The project, however, did not reach the stage where the OS could load other programs or execute commands. Despite this, I find the repository very interesting and well-documented.  

[Legacy documentation (in spanish) here](/docs/LEGACY.md)

### Download
Open a terminal on your desired directory and run:
```bash
git clone https://github.com/MauroCamerini/SOM8086.git
```
## Try It Yourself  
Because SOM8086 is an operating system, it needs to be installed on a disk image and run in a virtual machine. The repository includes binaries and disk images, so you can test it directly. *Back in the day it worked; recently I tried with QEMU and VirtualBox with bad results.*

### QEMU
With [QEMU](https://www.qemu.org/), it runs but inmmediatelly stops. I downloaded the [2023-08-22 QEMU installer (8.1.0)](https://qemu.weilnetz.de/w64/2023/qemu-w64-setup-20230822.exe) for security reasons, it's the last installer that has a valid certificate.
1. Download and install QEMU
2. Run:
```bash
cd "C:/Program Files/qemu"
./qemu-system-i386 -fda "%YOURFOLDER%/SOM8086/disk/DISCOM.IMA"
```
Where %YOURFOLDER% is the path to where you copied the repository.
### Virtual Box
I created a VirtualBox VM using the `disk/CDARNQ.ISO` image. Apparently it runs, but it does not capture keystrokes.

## Build Instructions
This project was developed on Windows, so the instructions provided are tailored to that environment.

### Required tools
- **`make`:** It is a tool which controls the generation of executables from the program's source files. It gets its knowledge of how to build your program from a file called the *makefile*.
Watch out the [main page of GNU Make](https://www.gnu.org/software/make/). You can get the Windows version on [Make for Windows](https://gnuwin32.sourceforge.net/packages/make.htm) or using [MinGW](https://www.mingw-w64.org/).
- **`wcc`:** [Open Watcom](https://www.openwatcom.org/) C compiler.  
- **`wlink`:** The linker provided with Watcom C/C++.  
- **`nasm`:** [NASM, the Netwide Assembler](https://github.com/netwide-assembler/nasm).  
**Ensure these programs are added to the `%PATH%`.**  

### Clone the repository
Open a terminal on your desired directory and run
```bash
git clone https://github.com/MauroCamerini/SOM8086.git
cd SOM8086
```
### Build `nucleo.bin` (Kernel)  
The `/src` directory contains a Makefile for compiling the kernel. Use the following command to build it:  
```bash
cd SOM8086/src
make
```  
It compiles `.asm` files with NASM and `.c` files with OpenWatcom into `.obj` files on the `obj` directory. Then it links everything into `nucleo.bin` file using WLINK. Compiler options ensure that only 8086 instructions are used, and the final binary is a pure binary file without a header.

#### NASM Options  
- `-f` Specifies the output format: OBJ.  

#### OpenWatcom `wcc` Options  
- `-0` Use 8086 instructions.  
- `-d0` Disable debugging information.  
- `-ms` Use "small" memory model (CS = ES = DS).  
- `-s` Disable stack overflow checks.  
- `-wx` Enable maximum warning level.  
- `-zl` Exclude library references in the object file.  
- `-i` Include directory for `.h` files.  

### `arranque.bin` (Bootloader)  
There is a `crear.bat` *(make)* script to build the bootloader. 
```bash
cd arranque
crear
``` 
It is compiled with NASM and translates the assembly code directly into its corresponding processor instructions. No linking or any other process is required. This routine is compiled into a 512-byte binary file.

### Utilities  
Two utilities are provided on the `utils` direcyory:  
1. **`insarnq`:** It is used to copy `arranque.bin` file into the boot sector of a specified disk image file.  
2. **`leernbp`:** Displays the boot sector information of a disk image file. 
These utilities are standalone tools and must be compiled directly on your PC's system as target, because they are not part of the OS. Here again, if you have [MinGW](https://www.mingw-w64.org/) installed you can run:
```bash
cd utils
gcc insarnq.c
gcc leernbp.c
``` 
---

### Instructions to install

1. Create a 3.5" 1.44MB floppy disk image. I used [WinImage](http://www.winimage.com) by Gilles Vollant, but the website seems broken. Other similar tools may work. *You can use the `leernbp` utility to check if the created image has the right format.*
2. Overwrite the disk image's boot sector with `arranque.bin` using the `insarnq` utility. 
```bash
insarnq arranque.bin floppy.ima
``` 
3. Copy the `nucleo.bin` file into the disk image. **It must be the very first file written to the disk** because the bootloader blindly loads the first file. It does not check the file system's indication of how the file fragments are stored on the disk.
4. Mount the disk image in a virtual machine.  
5. Enjoy!  