#!/usr/bin/env bash
#
# Set up the toolchain environment for RPV targets with Buildroot.
# The same script is used on a VM/PC and inside the Docker image.
#
# Missing host prerequisites are installed with apt-get (through sudo when
# not root), unless --no-install is given.

set -euo pipefail

ENV_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
. "$ENV_DIR/versions.env"

ALL_TARGETS=(x86_64 microblaze avr)

usage() {
    cat <<EOF
Usage: $(basename "$0") [options] <x86_64|microblaze|avr|all>...

Options:
  --prefix DIR   install location (default: \$RPV_ENV_PREFIX or ~/opt/rpv)
  --jobs N       parallel jobs per package (default: nproc)
  --clean        remove the Buildroot build directory after a successful build
  --no-env-check skip the final environment check
  --no-install   don't install packages: check them with env_check.sh,
                 then install any missing ones yourself
  --shell-rc     source env.sh from ~/.bashrc, so new shells have the toolchains
  -h, --help     show this help

Layout under the prefix:
  <target>/      toolchain (Buildroot host directory)
  bin/           symlinks to the toolchain executables, put this on PATH
  env.sh         source it to set PATH
  .cache/        Buildroot release and package downloads
  .build/        Buildroot build directories
EOF
}

die() {
    printf 'error: %s\n' "$*" >&2
    exit 1
}

log() {
    printf '\n==> %s\n' "$*"
}

PREFIX="${RPV_ENV_PREFIX:-$HOME/opt/rpv}"
JOBS="$(nproc)"
CLEAN=0
RUN_ENV_CHECK=1
INSTALL=1
SHELL_RC=0
TARGETS=()

while [ $# -gt 0 ]; do
    case "$1" in
        --prefix)     PREFIX="${2:?--prefix needs a directory}"; shift ;;
        --jobs)       JOBS="${2:?--jobs needs a number}"; shift ;;
        --clean)      CLEAN=1 ;;
        --no-env-check) RUN_ENV_CHECK=0 ;;
        --no-install) INSTALL=0 ;;
        --shell-rc)   SHELL_RC=1 ;;
        -h|--help)    usage; exit 0 ;;
        all)          TARGETS+=("${ALL_TARGETS[@]}") ;;
        x86_64|microblaze|avr) TARGETS+=("$1") ;;
        *)            usage >&2; die "unknown argument '$1'" ;;
    esac
    shift
done

[ ${#TARGETS[@]} -gt 0 ] || { usage >&2; exit 1; }

mkdir -p "$PREFIX"
PREFIX="$(cd "$PREFIX" && pwd)"

CACHE_DIR="$PREFIX/.cache"
BR_DIR="$CACHE_DIR/buildroot-$BUILDROOT_VERSION"

# Buildroot make targets to build, per target. Only host packages are
# built, so Buildroot never builds a Linux toolchain or a root filesystem.
packages_for() {
    case "$1" in
        x86_64)     echo "toolchain-bare-metal-buildroot host-nasm host-x86-64-elf-gdb" ;;
        microblaze) echo "toolchain-bare-metal-buildroot" ;;
        avr)        echo "host-avr8-gnu-toolchain" ;;
    esac
}

# Executables linked into $PREFIX/bin, per target. The Buildroot host
# directory also holds helper tools (m4, pkg-config, ...) that must not
# shadow the system ones, so it is not put on PATH as a whole.
tools_for() {
    case "$1" in
        x86_64)     echo "x86_64-elf-* nasm ndisasm" ;;
        microblaze) echo "microblaze-elf-*" ;;
        avr)        echo "avr-*" ;;
    esac
}

# Buildroot host prerequisites: command -> Debian/Ubuntu package
declare -A PREREQ_PKG=(
    [which]=debianutils [sed]=sed [file]=file [make]=make [gcc]=gcc [g++]=g++
    [bash]=bash [patch]=patch [gzip]=gzip [bzip2]=bzip2 [xz]=xz-utils
    [perl]=perl [tar]=tar [cpio]=cpio [unzip]=unzip [rsync]=rsync [bc]=bc
    [cmp]=diffutils [find]=findutils [xargs]=findutils [awk]=gawk
    [wget]=wget [sha256sum]=coreutils
)

# Print the packages of the missing prerequisites
missing_prereqs() {
    local cmd pkgs=()

    for cmd in "${!PREREQ_PKG[@]}"; do
        command -v "$cmd" >/dev/null 2>&1 || pkgs+=("${PREREQ_PKG[$cmd]}")
    done

    # perl-base has the interpreter but not the modules Buildroot needs
    if command -v perl >/dev/null 2>&1 && ! perl -MExtUtils::MakeMaker -e 1 2>/dev/null; then
        pkgs+=(perl)
    fi

    # wget needs the CA certificates for the https downloads
    if [ ! -f /etc/ssl/certs/ca-certificates.crt ]; then
        pkgs+=(ca-certificates)
    fi

    [ ${#pkgs[@]} -eq 0 ] || printf '%s\n' "${pkgs[@]}" | sort -u | paste -sd ' '
}

# Distribution packages the targets need to build, run and flash with make
runtime_packages_for() {
    case "$1" in
        x86_64)     echo "qemu-system-x86 ipxe-qemu grub-common grub-pc-bin xorriso mtools fdisk dosfstools" ;;
        microblaze) echo "" ;;
        avr)        echo "avrdude libexpat1 libstdc++6" ;;
    esac
}

# Print the missing runtime packages of the selected targets
missing_runtime() {
    local t pkg pkgs=()

    command -v dpkg-query >/dev/null 2>&1 || return 0

    for pkg in cmake make git openssh-client \
               $(for t in "${TARGETS[@]}"; do runtime_packages_for "$t"; done); do
        dpkg-query -W -f='${Status}' "$pkg" 2>/dev/null | grep -q 'install ok installed' \
            || pkgs+=("$pkg")
    done

    [ ${#pkgs[@]} -eq 0 ] || printf '%s\n' "${pkgs[@]}" | sort -u | paste -sd ' '
}

install_packages() {
    local sudo=()

    command -v apt-get >/dev/null 2>&1 || die "missing packages: $* (no apt-get, install them manually)"

    if [ "$(id -u)" -ne 0 ]; then
        command -v sudo >/dev/null 2>&1 || die "missing packages: $* (not root and no sudo)"
        sudo=(sudo)
    fi

    log "Installing missing packages: $*"
    "${sudo[@]}" apt-get update
    "${sudo[@]}" env DEBIAN_FRONTEND=noninteractive \
        apt-get install -y --no-install-recommends "$@"
}

# Buildroot prerequisites are required. Runtime packages are installed
# too, unless --no-install is given; env_check.sh then reports them.
check_prereqs() {
    local pkgs

    if [ "$INSTALL" -eq 1 ]; then
        pkgs="$(missing_prereqs) $(missing_runtime)"
        pkgs="${pkgs# }"; pkgs="${pkgs% }"

        if [ -n "$pkgs" ]; then
            # shellcheck disable=SC2086
            install_packages $pkgs
        fi

        pkgs="$(missing_runtime)"
        [ -z "$pkgs" ] || die "still missing after install: $pkgs"
    fi

    pkgs="$(missing_prereqs)"
    [ -z "$pkgs" ] || die "missing packages: $pkgs"

    if [[ " ${TARGETS[*]} " == *" avr "* ]] && [ "$(uname -m)" != "x86_64" ]; then
        die "the Microchip AVR toolchain is only available for x86_64 hosts"
    fi
}

# Buildroot refuses a PATH with relative entries or entries containing
# blanks, which WSL adds for Windows directories. Print a cleaned PATH.
# (LD_LIBRARY_PATH is also refused; it is unset where make is called.)
buildroot_path() {
    local parts dir clean=()

    IFS=: read -ra parts <<<"$PATH"
    for dir in "${parts[@]}"; do
        [[ "$dir" == /* && "$dir" != *[[:space:]]* ]] && clean+=("$dir")
    done

    (IFS=:; printf '%s' "${clean[*]}")
}

fetch_buildroot() {
    local tarball="$CACHE_DIR/buildroot-$BUILDROOT_VERSION.tar.xz"

    [ -f "$BR_DIR/Makefile" ] && return 0

    mkdir -p "$CACHE_DIR"

    if [ ! -f "$tarball" ]; then
        log "Downloading Buildroot $BUILDROOT_VERSION"
        wget -O "$tarball.part" "$BUILDROOT_SITE/buildroot-$BUILDROOT_VERSION.tar.xz"
        mv "$tarball.part" "$tarball"
    fi

    if ! echo "$BUILDROOT_SHA256  $tarball" | sha256sum -c --quiet; then
        rm -f "$tarball"
        die "Buildroot tarball checksum mismatch, removed it"
    fi

    tar -xJf "$tarball" -C "$CACHE_DIR"
}

build_target() {
    local t="$1"
    local out="$PREFIX/.build/$t"
    local host_dir="$PREFIX/$t"
    local br_path

    br_path="$(buildroot_path)"

    # Buildroot's stamp files would skip installing into a removed host dir
    if [ -d "$out" ] && [ ! -d "$host_dir" ]; then
        rm -rf "$out"
    fi

    log "Configuring $t (rpv_${t}_defconfig)"
    env -u LD_LIBRARY_PATH PATH="$br_path" \
        make -C "$BR_DIR" O="$out" BR2_EXTERNAL="$ENV_DIR/buildroot" "rpv_${t}_defconfig"

    "$BR_DIR/utils/config" --file "$out/.config" \
        --set-str BR2_HOST_DIR "$host_dir" \
        --set-str BR2_DL_DIR "$CACHE_DIR/dl" \
        --set-val BR2_JLEVEL "$JOBS"

    env -u LD_LIBRARY_PATH PATH="$br_path" make -C "$out" olddefconfig

    log "Building $t: $(packages_for "$t")"
    # shellcheck disable=SC2046
    env -u LD_LIBRARY_PATH PATH="$br_path" make -C "$out" $(packages_for "$t")

    if [ "$CLEAN" -eq 1 ]; then
        rm -rf "$out"
    fi
}

link_tools() {
    local t="$1" pattern f

    mkdir -p "$PREFIX/bin"

    for pattern in $(tools_for "$t"); do
        for f in "$PREFIX/$t/bin/"$pattern; do
            if [ -e "$f" ]; then
                ln -sfr "$f" "$PREFIX/bin/"
            fi
        done
    done
}

write_env() {
    cat > "$PREFIX/env.sh" <<EOF
# Generated by tools/env/setup.sh: source it to use the RPV toolchains
case ":\$PATH:" in
    *":$PREFIX/bin:"*) ;;
    *) export PATH="$PREFIX/bin:\$PATH" ;;
esac
EOF
}

# Remind what is left to do by hand after the toolchains are installed
RC_FILE="$HOME/.bashrc"

# Is env.sh sourced from ~/.bashrc (by --shell-rc now or on an earlier run)?
in_shell_rc() {
    [ -f "$RC_FILE" ] && grep -qF ". \"$PREFIX/env.sh\"" "$RC_FILE"
}

# Add a guarded line sourcing env.sh to ~/.bashrc, once
add_shell_rc() {
    in_shell_rc && return 0

    printf '\n[ -f "%s" ] && . "%s"  # RPV toolchains (tools/env/setup.sh)\n' \
        "$PREFIX/env.sh" "$PREFIX/env.sh" >> "$RC_FILE"
    log "Added env.sh to $RC_FILE"
}

next_steps() {
    local repo t

    repo="$(cd "$ENV_DIR/../.." && pwd)"

    cat <<EOF

==> Next steps
EOF

    if in_shell_rc; then
        cat <<EOF

  1. New shells get the toolchains from ~/.bashrc. In this shell run:
       . $PREFIX/env.sh
EOF
    else
        cat <<EOF

  1. Put the toolchains on PATH, in every new shell:
       . $PREFIX/env.sh
     or run setup.sh again with --shell-rc to do it from ~/.bashrc
EOF
    fi

    cat <<EOF

  2. The build fetches the kernel over git@github.com, check SSH access:
       ssh -T git@github.com

  3. Build from the repository root. Run 'make clean' first if build/ was
     configured with other toolchains (CMake caches the compiler path):
       cd $repo
EOF

    for t in "${TARGETS[@]}"; do
        case "$t" in
            x86_64) cat <<EOF

  [x86_64]
       make x86_64-debug         kernel ELF
       make x86_64-iso           bootable GRUB ISO
       make x86_64-run           boot in QEMU (quit: Esc, 2, then 'quit')
       make x86_64-qemu-debug    QEMU waits for gdb on :1234, then in another shell:
         x86_64-elf-gdb build/x86_64-debug/bin/rpv-x86_64-kernel
EOF
            ;;
            microblaze) cat <<EOF

  [microblaze]
       make microblaze_le-debug  kernel ELF and raw binary
     The build also fetches rpv-security-guardian and FatFs over SSH.
EOF
            ;;
            avr) cat <<EOF

  [avr]
       make microchip_avr-debug  kernel ELF/HEX
       make microchip_avr-prog   flash the Arduino board on /dev/ttyACM0
     Flashing needs the user in the serial port group (log in again
     afterwards):
       sudo usermod -aG dialout \$USER
EOF
            ;;
        esac
    done
}

check_prereqs
fetch_buildroot

for t in "${TARGETS[@]}"; do
    build_target "$t"
    link_tools "$t"
done

write_env

if [ "$SHELL_RC" -eq 1 ]; then
    add_shell_rc
fi

status=0

if [ "$RUN_ENV_CHECK" -eq 1 ]; then
    log "Checking the environment"
    "$ENV_DIR/env_check.sh" --prefix "$PREFIX" "${TARGETS[@]}" || status=$?
fi

next_steps
exit "$status"
