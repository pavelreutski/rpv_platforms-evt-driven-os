# PHONY targets
.PHONY: x86_64-debug clean run

.EXPORT_ALL_VARIABLES:
MAKEFLAGS = --no-print-directory

# Build: configure if needed and build via CMake
x86_64-debug:
	cmake --preset x86_64-debug
	cmake --build --preset x86_64-debug

# Build and generate kernel ISO
x86_64-iso: x86_64-debug
	cmake --build --preset x86_64-debug --target iso

x86_64-run: x86_64-iso
	cmake --build --preset x86_64-debug --target qemu-x86_64-run

x86_64-qemu-debug: x86_64-iso
	cmake --build --preset x86_64-debug --target qemu-x86_64-gdb

# Clean build directory via CMake
clean:
	rm -rf build
# cmake --build --preset x86_64-debug --target clean
