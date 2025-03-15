# Project structure
## **src/hard**
Plateform definitions

## **src/soft** 
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
