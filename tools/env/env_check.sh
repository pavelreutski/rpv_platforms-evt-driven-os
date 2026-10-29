#!/usr/bin/env bash
#
# Check the environment of RPV targets. Changes nothing.
#
# Failures are things the build needs. Warnings are things that only some
# make targets need (network, serial port).

set -uo pipefail

ALL_TARGETS=(x86_64 microblaze avr)

usage() {
    cat <<EOF
Usage: $(basename "$0") [--prefix DIR] <x86_64|microblaze|avr|all>...

  --prefix DIR   also look for tools in DIR/bin (default: \$RPV_ENV_PREFIX or ~/opt/rpv)
EOF
}

PREFIX="${RPV_ENV_PREFIX:-$HOME/opt/rpv}"
TARGETS=()

while [ $# -gt 0 ]; do
    case "$1" in
        --prefix)  PREFIX="${2:?--prefix needs a directory}"; shift ;;
        -h|--help) usage; exit 0 ;;
        all)       TARGETS+=("${ALL_TARGETS[@]}") ;;
        x86_64|microblaze|avr) TARGETS+=("$1") ;;
        *)         usage >&2; exit 1 ;;
    esac
    shift
done

[ ${#TARGETS[@]} -gt 0 ] || { usage >&2; exit 1; }

# Works before env.sh is sourced
if [ -d "$PREFIX/bin" ]; then
    PATH="$PREFIX/bin:$PATH"
fi

FAILS=0
WARNS=0

ok()   { printf '  ok    %s\n' "$*"; }
bad()  { printf '  FAIL  %s\n' "$*"; FAILS=$((FAILS + 1)); }
warn() { printf '  warn  %s\n' "$*"; WARNS=$((WARNS + 1)); }

# version_ge 3.31.6 3.25
version_ge() {
    [ "$(printf '%s\n%s\n' "$2" "$1" | sort -V | head -n1)" = "$2" ]
}

need() {
    local cmd

    for cmd in "$@"; do
        if command -v "$cmd" >/dev/null 2>&1; then
            ok "$cmd"
        else
            bad "$cmd not found"
        fi
    done
}

# check_gcc <compiler> [flags...]: version >= 9.1 (CMake c_std_23) and a C23 compile
check_gcc() {
    local cc="$1" ver
    shift

    if ! command -v "$cc" >/dev/null 2>&1; then
        bad "$cc not found"
        return
    fi

    ver="$("$cc" -dumpfullversion 2>/dev/null)"
    if version_ge "$ver" 9.1; then
        ok "$cc $ver"
    else
        bad "$cc $ver is older than 9.1 (no C23 in CMake)"
    fi

    if printf 'static_assert(true);\nint f(void) { return 0; }\n' \
        | "$cc" "$@" -std=c23 -x c -c - -o /dev/null 2>/dev/null; then
        ok "$cc compiles C23"
    else
        bad "$cc cannot compile C23 with: $*"
    fi
}

check_common() {
    local ver out

    printf '\n[common]\n'

    need make git ssh

    if command -v cmake >/dev/null 2>&1; then
        ver="$(cmake --version | sed -n '1s/^cmake version //p')"
        if version_ge "$ver" 3.25; then
            ok "cmake $ver"
        else
            bad "cmake $ver is older than 3.25"
        fi
    else
        bad "cmake not found"
    fi

    # Kernel, guardian and FatFs are fetched over git@github.com
    out="$(ssh -T -o BatchMode=yes -o ConnectTimeout=10 git@github.com 2>&1)"
    if [[ "$out" == *"successfully authenticated"* ]]; then
        ok "ssh git@github.com"
    else
        warn "ssh git@github.com failed (FetchContent needs it): ${out%%$'\n'*}"
    fi
}

check_x86_64() {
    printf '\n[x86_64]\n'

    check_gcc x86_64-elf-gcc -m64 -ffreestanding
    need x86_64-elf-ld x86_64-elf-objcopy x86_64-elf-objdump x86_64-elf-gdb nasm

    # libc headers only, from newlib
    if printf '#include <string.h>\n#include <stdlib.h>\n#include <assert.h>\n' \
        | x86_64-elf-gcc -E -x c - -o /dev/null 2>/dev/null; then
        ok "newlib headers"
    else
        bad "x86_64-elf-gcc does not find the newlib headers"
    fi

    need grub-mkrescue xorriso mcopy mkfs.vfat sfdisk

    if [ -d /usr/lib/grub/i386-pc ]; then
        ok "grub i386-pc modules"
    else
        bad "grub i386-pc modules missing (/usr/lib/grub/i386-pc, grub-pc-bin)"
    fi

    if command -v qemu-system-x86_64 >/dev/null 2>&1; then
        if qemu-system-x86_64 -display help 2>/dev/null | grep -qx curses; then
            ok "qemu-system-x86_64 with curses"
        else
            bad "qemu-system-x86_64 has no curses display (needed by make x86_64-run)"
        fi
    else
        bad "qemu-system-x86_64 not found"
    fi
}

check_microblaze() {
    local flags=(-mcpu=v11.0 -mlittle-endian -mxl-soft-mul) dir lib

    printf '\n[microblaze]\n'

    check_gcc microblaze-elf-gcc "${flags[@]}" -ffreestanding
    need microblaze-elf-ld microblaze-elf-objcopy microblaze-elf-objdump

    command -v microblaze-elf-gcc >/dev/null 2>&1 || return

    dir="$(microblaze-elf-gcc "${flags[@]}" -print-multi-directory 2>/dev/null)"
    if [ -n "$dir" ] && [ "$dir" != "." ]; then
        ok "little-endian multilib ($dir)"
    else
        bad "no little-endian multilib for: ${flags[*]}"
    fi

    for lib in libc.a libgcc.a; do
        if [[ "$(microblaze-elf-gcc "${flags[@]}" -print-file-name="$lib")" == /* ]]; then
            ok "$lib ($dir)"
        else
            bad "$lib missing for the $dir multilib"
        fi
    done
}

check_avr() {
    local port="${RPV_AVR_PORT:-/dev/ttyACM0}"

    printf '\n[avr]\n'

    check_gcc avr-gcc -mmcu=atmega328p
    need avr-objcopy avr-objdump avr-size avrdude

    # avr-gdb is prebuilt and needs libexpat and libstdc++ on the host
    if ! command -v avr-gdb >/dev/null 2>&1; then
        bad "avr-gdb not found"
    elif avr-gdb --version >/dev/null 2>&1; then
        ok "avr-gdb"
    else
        bad "avr-gdb does not run (missing libexpat1 or libstdc++6?)"
    fi

    if [ ! -e "$port" ]; then
        warn "$port not present (only needed by make microchip_avr-prog)"
    elif [ -r "$port" ] && [ -w "$port" ]; then
        ok "$port accessible"
    else
        warn "$port not accessible, add the user to group '$(stat -c %G "$port")'"
    fi
}

check_common

for t in "${TARGETS[@]}"; do
    "check_$t"
done

printf '\n%d failure(s), %d warning(s)\n' "$FAILS" "$WARNS"
[ "$FAILS" -eq 0 ]
