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
tools/env/                environment setup: Buildroot toolchains, env check, Docker image
targets/x86_64/iso/       GRUB ISO tree (grub.cfg)
```

## Requirements

`tools/env/setup.sh` sets up everything listed below for a target (see
[Environment setup](#environment-setup)). The lists document what a manual setup needs.

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
| `sfdisk` (`fdisk` package), `dosfstools`, `mtools` | FAT16 IDE disk image for QEMU |
| `qemu-system-x86_64` **with curses support (required)** | run the kernel |
| `x86_64-elf-gdb` (optional) | debugging |

Debian packages: `build-essential`, `nasm`, `grub-common`, `grub-pc-bin`, `xorriso`, `mtools`,
`fdisk`, `dosfstools`, `qemu-system-x86`.

#### QEMU with curses support

> [!CAUTION]
> **QEMU must be built with curses support.** The run and debug targets start QEMU with
> `-display curses`, which draws the VGA text console in the terminal, with no GUI needed.
> Without curses support, QEMU fails to start. Debian's `qemu-system-x86` package includes it.
> If QEMU is installed from sources, it must be compiled with `--enable-curses`.
> Check with `qemu-system-x86_64 -display help`; the list must contain `curses`.

### Microchip AVR

- `avr-gcc` 9.1 or newer, `avr-binutils`, `avr-libc` (AVR-LibC)
- `avrdude`. It's needed even to configure, because the CMake files look for it.
- An Arduino-compatible board on `/dev/ttyACM0` for flashing

> [!CAUTION]
> **The distribution's `gcc-avr` package is too old.** The AVR target is compiled as C23
> (`c_std_23`), and CMake supports C23 only for GCC 9.1 and newer. Debian's `gcc-avr` is GCC 7.3,
> so CMake configure fails with `The compiler feature "c_std_23" is not known`.
> avr-gcc must be built from sources (GCC 15.3.0 with AVR-LibC 2.3.2 is tested).
> Check with `avr-gcc --version`.

### MicroBlaze

- `microblaze-elf` toolchain: binutils, GCC 9.1 or newer (C23) and `libgcc`, with the
  little-endian multilib (`-mlittle-endian`)
- The C standard library is taken from **newlib** (`libc`)
- No Xilinx SDK/BSP is used. The startup code, interrupt vectors, linker script and drivers are all
  part of this repository.

> [!CAUTION]
> **AMD/Xilinx Vitis is not used, for licensing reasons.** The toolchain is built from sources.
> Vitis' compiler is also named `microblaze-xilinx-elf-*`, not `microblaze-elf-*`.

## Environment setup

There are three ways to set up the environment:

| Where | How |
|---|---|
| Linux (Debian/Ubuntu PC or VM) | `tools/env/setup.sh`, see below |
| Any Debian/Ubuntu Linux container you manage yourself (for example from `ubuntu:24.04`) | `tools/env/setup.sh` inside the container, the same as on Linux |
| Windows | only through the [docker shell](#docker): `docker_shell.ps1` / `docker_shell.cmd`. Setting up or building directly on Windows isn't supported. |

The docker shell also works on Linux and WSL (`docker_shell.sh`).

`tools/env/setup.sh` builds the cross toolchains with [Buildroot](https://buildroot.org)
and installs the distribution packages a target needs. The docker shell images are built with
the same script.

```bash
tools/env/setup.sh x86_64          # or microblaze, avr, all
. ~/opt/rpv/env.sh                 # put the toolchains on PATH
```

| Target | Toolchain |
|---|---|
| `x86_64` | binutils, GCC (C only) and newlib for `x86_64-elf`, `x86_64-elf-gdb`, nasm |
| `microblaze` | binutils, GCC (C only, multilib) and newlib for `microblaze-elf` |
| `avr` | Microchip AVR 8-bit GNU toolchain (prebuilt: avr-gcc, AVR-LibC, avr-gdb), x86_64 hosts only |

Notes:

- The toolchains are installed in `~/opt/rpv/<target>`, with symlinks to the tools in
  `~/opt/rpv/bin`. Use `--prefix DIR` (or `RPV_ENV_PREFIX`) to install them elsewhere.
- Missing packages are installed with `apt-get`, through `sudo` when not root.
  With `--no-install`, no packages are installed: check them with `tools/env/env_check.sh`, then
  install any missing ones yourself.
- SSH keys are not set up by the environment tools. The builds fetch the kernel, security guardian
  and FatFs over `git@github.com:...`, so install a key manually before building:
  generate it with `ssh-keygen -t ed25519`, add the public key to your GitHub account, and check
  with `ssh -T git@github.com`. In a container, see [Docker](#docker) for how keys are passed in.
- `env.sh` has to be sourced in each new shell. `--shell-rc` adds it to `~/.bashrc`.
- Versions are pinned in `tools/env/versions.env` (Buildroot release) and in the custom packages
  under `tools/env/buildroot/package/`.
- After switching toolchains (for example between the host and the container), run `make clean`.
  CMake caches the compiler path.

`tools/env/env_check.sh <target>` checks an environment without changing anything: compilers and
C23 support, MicroBlaze multilib, QEMU curses support, GRUB, SSH access to GitHub and the AVR
serial port. `tools/env/setup.sh` runs it at the end, unless `--no-env-check` is given.

### Docker

`tools/env/docker/Dockerfile` builds one image per docker shell target. The images are named
`rpv-env:<target>`:

| Docker shell target | Image | Contents | Make targets inside |
|---|---|---|---|
| `x86_64` | `rpv-env:x86_64` | x86_64 toolchain and gdb, nasm, QEMU, GRUB, xorriso, mtools, fdisk, dosfstools | `x86_64-debug`, `x86_64-iso`, `x86_64-run`, `x86_64-qemu-debug` |
| `microblaze` | `rpv-env:microblaze` | MicroBlaze toolchain | `microblaze_le-debug` |
| `avr` | `rpv-env:avr` | AVR toolchain and avr-gdb, avrdude | `microchip_avr-debug`, `microchip_avr-prog` |
| `all` | `rpv-env:all` | all of the above | all of the above |

The `docker_shell` scripts build the image of a target if it doesn't exist yet, then run a
container from it with the repository mounted. The container is removed when it exits.

| Host | Script | Repository in the container |
|---|---|---|
| Linux, WSL | `tools/env/docker/docker_shell.sh` | same path as on the host |
| Windows | `tools\env\docker\docker_shell.ps1`, or `docker_shell.cmd` if the PowerShell execution policy blocks scripts | `/work` |

All scripts take the same arguments: `[--rebuild] <x86_64|microblaze|avr|all> [command...]`.
Without a command, they start an interactive shell.

```bash
tools/env/docker/docker_shell.sh x86_64                     # interactive shell
tools/env/docker/docker_shell.sh x86_64 make x86_64-iso     # single command
tools/env/docker/docker_shell.sh --rebuild x86_64           # rebuild the image first
```

```powershell
tools\env\docker\docker_shell.ps1 x86_64
tools\env\docker\docker_shell.ps1 x86_64 make x86_64-iso
tools\env\docker\docker_shell.ps1 --rebuild x86_64
```

SSH keys for the GitHub fetches are never stored in the image:

- `docker_shell.sh` forwards the host's ssh-agent.
- Without a forwarded agent, the container starts its own. Load a key with `ssh-add <key>`.
  `docker_shell.ps1` mounts `%USERPROFILE%\.ssh` read-only at `/mnt/ssh`:

  ```bash
  install -Dm600 /mnt/ssh/<key> ~/.ssh/<key> && ssh-add ~/.ssh/<key>
  ```

  Docker Desktop reads the mounted files as your Windows user, with a non-elevated token. If the
  key's ACL grants access to Administrators only, the container gets "permission denied". Grant
  your own user read access:

  ```powershell
  icacls "$env:USERPROFILE\.ssh" /grant "${env:USERNAME}:(RX)"
  icacls "$env:USERPROFILE\.ssh\<key>" /grant "${env:USERNAME}:(R)"
  ```

For AVR, `docker_shell.sh` passes the serial port into the container (`RPV_AVR_PORT`, default
`/dev/ttyACM0`). On Windows, passing USB devices to Docker isn't supported by the script:
flash from a Linux or WSL host.

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
| `make microblaze_le-debug` | MicroBlaze kernel ELF (little-endian, entry point `0x82000000` in DDR) and raw binary |

## Running (x86_64)

```bash
make x86_64-run
```

This builds the ISO, creates a 20 MB FAT16 IDE disk image with a `test.txt` file, and boots it
in QEMU. The kernel shell prompt `C:/>` appears in the terminal. Try `dir`.

To quit QEMU, press `Alt+2` (or `Esc` then `2`) and type `quit`. Alternatively, run
`pkill qemu-system-x86_64` from another shell.

> [!CAUTION]
> **Expect one host CPU core at full load while the x86_64 kernel runs in QEMU.** The kernel is
> an event-driven real-time kernel whose main loop polls without pause, and it never halts the
> CPU when idle. The x86_64 port exists for tests and academic purposes in QEMU, not for real
> x86_64 hardware. QEMU runs the guest CPU in one host thread, which then never goes idle. On a
> modern desktop CPU (observed on an AMD Ryzen 5 9600X) the loop runs very fast, so that core
> stays at full load and its temperature rises. This is expected for now and applies to both
> `make x86_64-run` and `make x86_64-qemu-debug`. Reducing the idle load of the x86_64 port
> (for example by halting the CPU until the next interrupt when there is no work) will be
> investigated in the future.

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
