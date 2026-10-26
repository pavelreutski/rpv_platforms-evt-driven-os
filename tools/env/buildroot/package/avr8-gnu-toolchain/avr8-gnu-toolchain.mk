################################################################################
#
# avr8-gnu-toolchain
#
################################################################################

# 4.0.0.52: GCC 15.1.0, AVR-LibC 2.2.1, avr-gdb
AVR8_GNU_TOOLCHAIN_VERSION = 4.0.0.52
AVR8_GNU_TOOLCHAIN_SITE = https://ww1.microchip.com/downloads/aemDocuments/documents/DEV/ProductDocuments/SoftwareTools
AVR8_GNU_TOOLCHAIN_SOURCE = avr8-gnu-toolchain-$(AVR8_GNU_TOOLCHAIN_VERSION)-linux.any.x86_64.tar.gz
AVR8_GNU_TOOLCHAIN_LICENSE = GPL-3.0+, LGPL-3.0+, BSD-3-Clause (AVR-LibC)

# Installed in its own directory and only the executables are linked into
# $(HOST_DIR)/bin, so the prebuilt libraries don't mix with the ones
# Buildroot builds for the host. gcc resolves the symlinks to find its
# own libexec and library directories.
HOST_AVR8_GNU_TOOLCHAIN_INSTALL_DIR = $(HOST_DIR)/opt/avr8-gnu-toolchain

define HOST_AVR8_GNU_TOOLCHAIN_INSTALL_CMDS
	rm -rf $(HOST_AVR8_GNU_TOOLCHAIN_INSTALL_DIR)
	mkdir -p $(HOST_AVR8_GNU_TOOLCHAIN_INSTALL_DIR) $(HOST_DIR)/bin
	cp -a $(@D)/* $(HOST_AVR8_GNU_TOOLCHAIN_INSTALL_DIR)/
	cd $(HOST_DIR)/bin && \
		for f in ../opt/avr8-gnu-toolchain/bin/avr-*; do \
			ln -sf $$f . || exit 1; \
		done
endef

$(eval $(host-generic-package))
