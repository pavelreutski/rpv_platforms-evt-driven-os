# RPV Event Driven OS: Platforms

Platform targets (board support, drivers and services) for the
[RPV event-driven kernel](https://github.com/pavelreutski/rpv-evt-driven-os).

The kernel itself is not stored here. Each target fetches it at configure time with CMake
`FetchContent` and links it together with the target's startup code, hardware drivers,
services and shell commands.

| Target | CPU / board | Highlights |
|---|---|---|
| `x86_64` | PC / QEMU | multiboot2 + GRUB ISO, long mode, LAPIC/IOAPIC, PS/2 keyboard, PATA disk, VGA text console, FatFs |
| `microchip_avr` | ATmega328p @ 16 MHz (Arduino) | GPIO, SPI, UART, wheels counter service |
| `microblaze_le` | Xilinx MicroBlaze (little endian) | INTC, timer, UART, TEMAC Ethernet + lwIP, DMA, SD card + FatFs, sysmon temperature sensor, security guardian |

## Repository layout

```
CMakeLists.txt, CMakePresets.json, Makefile   build entry points
src/
  CMakeLists.txt          dispatches on KERNEL_TARGET
  arch/<arch>/            startup, console, tty, prompt
    sys/                  low-level hardware drivers
    services/             kernel services and shell commands
    target.cmake          per-target build (fetches the kernel)
  include/<arch>/         per-target headers
  scripts/                linker scripts
  fatfs/, lwip/           third-party library glue
tools/cmake/              toolchain files and build helpers (ISO, QEMU, elf utils, AVR)
targets/x86_64/iso/       GRUB ISO tree (grub.cfg)
```

## Requirements

Common to all targets:

- CMake 3.25 or newer (3.20 is enough for x86_64 and MicroBlaze) and GNU make
- git with SSH access to GitHub. The kernel, security guardian and FatFs are fetched
  over `git@github.com:...`, and lwIP is fetched from savannah.

### x86_64

| Tool | Purpose |
|---|---|
| `x86_64-elf-gcc`, `x86_64-elf-binutils` | bare-metal cross compiler (on `PATH`) |
| newlib headers | libc headers only (`string.h`, `stdlib.h`, `assert.h`), no libc is linked |
| `nasm` | boot and low-level assembly |
| `grub-mkrescue`, `grub-pc-bin`, `xorriso`, `mtools` | bootable ISO |
| `parted`, `dosfstools`, `mtools` | FAT16 IDE disk image for QEMU |
| `qemu-system-x86_64` **with curses support (required)** | run the kernel |
| `x86_64-elf-gdb` (optional) | debugging |

Debian packages: `build-essential`, `nasm`, `grub-common`, `grub-pc-bin`, `xorriso`, `mtools`,
`parted`, `dosfstools`, `qemu-system-x86`.

#### QEMU with curses support

> [!CAUTION]
> **QEMU must be built with curses support.** The run and debug targets start QEMU with
> `-display curses`, which draws the VGA text console in the terminal, with no GUI needed.
> Without curses support, QEMU fails to start. Debian's `qemu-system-x86` package includes it.
> If QEMU is installed from sources, it must be compiled with `--enable-curses`.
> Check with `qemu-system-x86_64 -display help`; the list must contain `curses`.

### Microchip AVR

- `avr-gcc`, `avr-binutils`, `avr-libc`
- `avrdude`. It's needed even to configure, because the CMake files look for it.
- An Arduino-compatible board on `/dev/ttyACM0` for flashing

### MicroBlaze

- `microblaze-elf-gcc` toolchain with newlib (`libc`) and `libgcc`, for example from AMD/Xilinx Vitis

## Building

The build uses CMake presets, and the `Makefile` wraps them. Output goes to `build/<preset>/bin`.
`make clean` removes `build/`.

### x86_64

| Command | Result |
|---|---|
| `make x86_64-debug` | x86_64 kernel ELF |
| `make x86_64-iso` | bootable GRUB ISO `build/x86_64-debug/bin/rpv-x86_64-kernel.iso` |
| `make x86_64-run` | build the ISO and boot it in QEMU (curses display in the terminal) |
| `make x86_64-qemu-debug` | build the ISO and start QEMU paused, waiting for gdb on port 1234 |

### Microchip AVR

| Command | Result |
|---|---|
| `make microchip_avr-debug` | AVR kernel ELF/HEX |
| `make microchip_avr-prog` | build and flash the Arduino board with avrdude |

### MicroBlaze

| Command | Result |
|---|---|
| `make microblaze_le-debug` | MicroBlaze kernel ELF |

## Running (x86_64)

```bash
make x86_64-run
```

This builds the ISO, creates a 20 MB FAT16 IDE disk image with a `test.txt` file, and boots it
in QEMU. The kernel shell prompt `C:/>` appears in the terminal. Try `dir`.

To quit QEMU, press `Alt+2` (or `Esc` then `2`) and type `quit`. Alternatively, run
`pkill qemu-system-x86_64` from another shell.

## Debugging (x86_64)

Terminal 1 starts QEMU paused, waiting for gdb on port 1234:

```bash
make x86_64-qemu-debug
```

Terminal 2 attaches gdb:

```bash
x86_64-elf-gdb build/x86_64-debug/bin/rpv-x86_64-kernel
```

```gdb
(gdb) set architecture i386:x86-64
(gdb) target remote :1234
(gdb) hbreak _long_mode_start      # first 64-bit instruction
(gdb) hbreak _cstart_entry         # C entry point
(gdb) continue
```

Notes:

- **Use `hbreak` (hardware breakpoints), not `break`.** GRUB loads the kernel after gdb has
  attached and overwrites normal breakpoints. The CPU has 4 hardware breakpoints.
- **QEMU accepts one gdb connection at a time.** `detach` lets the guest continue running.
- **Kernel sources are available.** gdb finds them under
  `build/x86_64-debug/_deps/evtdriven-kernel-src/`, so source-level stepping works there too.
