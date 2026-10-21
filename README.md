# 🖥️ CalcOS

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
![Architecture](https://img.shields.io/badge/arch-x86-orange.svg)
![Architecture](https://img.shields.io/badge/arch-RISC--V-orange.svg)

## ✨ Features

- **Multi-architecture support:** Runs on both `x86` and `RISC-V` (RV32IMAC).
- **Custom Bootloader & Kernel:** Bare-metal initialization without standard libraries (freestanding).
- **File System:** FAT12 support for reading and writing files.
- **Networking:** Integrated network stack (ARP, IP, ICMP) with `RTL8139` driver support.
- **User Space & Binaries:** Custom executable loading (`.BIN` apps like `LS.BIN`, `LSPCI.BIN`).
- **Graphics & UI:** Basic GUI / Desktop environment with windowing and 3D graphics rendering capabilities (`Calc3D`).
- **Shell & Commands:** Interactive command-line utilities.

---

## 🛠️ Prerequisites

To build and run this project, you will need the following tools (tested on Linux/Ubuntu):

*   **Build tools:** `build-essential`, `gcc`, `binutils`, `make`
*   **Assemblers:** `nasm` (for x86)
*   **Disk & Image tools:** `mtools`, `dd`
*   **Emulators:** `qemu-system-i386`, `qemu-system-riscv32`, `bochs` (optional)
*   *For RISC-V:* `riscv64-unknown-elf-gcc` toolchain.

---

## 🚀 Getting Started & Build Commands

This project uses a versatile `Makefile` to handle multiple architectures. Here are the most useful commands:

| Command | Description |
| :--- | :--- |
| `make` | Builds the complete floppy disk image (`os-image.img`) for **x86**. |
| `make ARCH=riscv` | Compiles the kernel for **RISC-V**. |
| `make run` | Builds the x86 image and runs it in **QEMU** (with `rtl8139` network support). |
| `make rear` | Cleans, builds, and runs the **RISC-V** version in QEMU (`virt` machine with `virtio-gpu`). |
| `make boch` | Launches the x86 image using the **Bochs** emulator. |
| `make clean` | Removes temporary object files and binaries. |
| `make cleane` | Performs a full clean (removes disk images and network logs). |
| `make help` | Displays a full list of available Makefile commands. |

### Quick Run Example (x86):
```bash
# Clone the repository
git clone https://github.com/O-V-Soft/calc-os.git
cd calc-os

# Build and run in QEMU
make run

