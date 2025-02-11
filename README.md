# ![KO6 Logo](docs/logo55.png) ko6

```
 Small Operating System for Educational Purpose
  _     ___    __
 | |__ /'v'\  / /      \date        2025-02-11
 | / /(     )/ _ \     \copyright   2021-2025 Sorbonne University
 |_\_\ x___x \___/                  https://opensource.org/licenses/MIT
```

## License

* ko6 is licensed under the **MIT License**.
* Some third-party components, such as `external/libfdt/`, are licensed under the **BSD-2-Clause** License.
* See the [LICENSE](./LICENSE) and [LICENSE.BSD](./src/soft/external/libfdt/LICENSE.BSD) files for full details.
* If you use ko6, let me know! I'll be happy to hear about it and help if I can.

## Project Authors

See the full list of contributors in [CONTRIBUTORS.md](./CONTRIBUTORS.md).

## Presentation

ko6 (*pronounce it "kit-o-sys" o is  the letter not the number*) is a small operating system for educational purpose. 

## Configuration

If you put ko6 in your home directory, you must source the `SourceMe.sh` file to set the environment. You can add `source $HOME/ko6/SourceMe.sh` in the `$HOME/.bashrc`. Once, the environment is set, in order to check if it works, run : `cd $HOME/ko6; make hello`, you must see two xterm windows appear. On the big one (xterm0), you must see, the logo and a system dump (internal registers and scheduler). On the small one, you must see « Hello World! ».

The source files are all 100 characters long because of the many comments on the lines. Therefore, 100-character wide windows must be used for editing. The C code uses the Kernighan and Ritchie style.

## KO6 Project Structure

### **src/hard**
Plateform definitions

### **src/soft** 
Code of Operating System and Applications

* **common/**<br>
  Shared files between applications and the kernel.

* **hal/**<br>
  Hardware Abstraction Layer (HAL) API declarations and implementations for each hardware.

  - **cpu/**
    - **mips/**: Implementation for MIPS.
    - **riscv/**: Implementation for RISC-V.

  - **devices/**<br>
    API for different types of devices.<br>
    Some device types are missing, particularly block devices.<br>
    Each device type may have multiple implementations located in subdirectories.
    - **chardev/**: Character devices.
    - **dma/**: Direct Memory Access.
    - **icu/** (*could be renamed to irq*): Interrupt Controller Unit.
    - **timer/**: Timer management.

  - **soc/**<br>
    A SoC consists of a CPU and various devices.<br>
    The API here is limited to initialization, which takes the device tree as an argument.<br>
    The initialization function is implemented differently for each SoC, calling CPU and device initialization functions.
    - **almo1-mips/**: SoC implementation for ALMO1-MIPS.
    - **qemu-virt-riscv/**: SoC implementation for QEMU-Virt-RISC-V.

* **kernel/**<br>
  Kernel service code, independent of hardware resources.<br>
  The number of files is intentionally minimized, with one C file per subsystem.

* **fs/**<br>
  Code related to real file systems.<br>
  Includes both the implementation of Virtual File System (VFS) functions inside the kernel and tools for constructing the disk.

  - **kfs/**: Implementation of the KFS real file system.

* **external/**<br>
  Third-party code.

  - **libfdt/**: Flat Device Tree (FDT) library.

* **ulib/**<br>
  User-space libraries.

* **uapp/**<br>
  User applications.

  - **barrier/**
  - **hello/**
  - **test/**
