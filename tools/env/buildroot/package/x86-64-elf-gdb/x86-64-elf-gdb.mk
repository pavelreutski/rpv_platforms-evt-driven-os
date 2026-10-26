################################################################################
#
# x86-64-elf-gdb
#
################################################################################

X86_64_ELF_GDB_VERSION = 17.1
X86_64_ELF_GDB_SITE = $(BR2_GNU_MIRROR)/gdb
X86_64_ELF_GDB_SOURCE = gdb-$(X86_64_ELF_GDB_VERSION).tar.xz
X86_64_ELF_GDB_LICENSE = GPL-2.0+, LGPL-2.0+, GPL-3.0+, LGPL-3.0+
X86_64_ELF_GDB_LICENSE_FILES = COPYING COPYING.LIB COPYING3 COPYING3.LIB

HOST_X86_64_ELF_GDB_DEPENDENCIES = \
	host-expat \
	host-gmp \
	host-mpfr \
	host-ncurses \
	host-readline \
	host-zlib

# Same options as Buildroot's host-gdb, but for the bare-metal target
HOST_X86_64_ELF_GDB_CONF_OPTS = \
	--target=x86_64-elf \
	--enable-static \
	--disable-shared \
	--without-uiout \
	--disable-gdbtk \
	--without-x \
	--enable-threads \
	--disable-werror \
	--without-included-gettext \
	--with-expat \
	--with-system-zlib \
	--with-curses \
	--enable-tui \
	--disable-source-highlight \
	--with-system-readline \
	--without-python \
	--disable-sim \
	--without-lzma \
	--without-xxhash \
	--disable-binutils \
	--disable-install-libbfd \
	--disable-ld \
	--disable-gas \
	--disable-gprof \
	--with-mpfr=$(HOST_DIR)

HOST_X86_64_ELF_GDB_MAKE_OPTS = MAKEINFO=true
HOST_X86_64_ELF_GDB_INSTALL_OPTS = MAKEINFO=true install

# In-tree builds are not allowed, so configure from a 'build' subdirectory
HOST_X86_64_ELF_GDB_SUBDIR = build

define HOST_X86_64_ELF_GDB_CONFIGURE_SYMLINK
	mkdir -p $(@D)/build
	ln -sf ../configure $(@D)/build/configure
endef
HOST_X86_64_ELF_GDB_PRE_CONFIGURE_HOOKS += HOST_X86_64_ELF_GDB_CONFIGURE_SYMLINK

$(eval $(host-autotools-package))
