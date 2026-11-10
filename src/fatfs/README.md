
# FatFs

Vendored FatFs a lightweight FAT/exFAT filesystem implementation.

## Upstream

- Project: FatFs
- Repository Mirror: https://github.com/abbrev/fatfs

## Directory structure

- `upstream/` — Vendored upstream FatFs source snapshot.
- `bridge/` — Specific integration between the kernel
  filesystem subsystem and FatFs.
- `config/ffconf.h.in` — RPV-specific FatFs configuration template.

## Integration

FatFs is integrated through the project CMake
build system.

The `bridge/` component provides the interface between
the kernel filesystem subsystem and FatFs.

It is responsible for adapting filesystem operations
and connecting FatFs to the underlying services.

The configuration template is used to generate `ffconf.h`
in the build output directory.

## Source ownership

Upstream FatFs sources are vendored and maintained locally
within this repository.

Specific integration code and configuration are
maintained separately from upstream sources.

Upstream updates must be explicitly reviewed and integrated.
The upstream source snapshot should remain unmodified
whenever possible.

## Configuration policy

The current FatFs configuration is shared across targets.

Target-specific configurations may be introduced when
required by platform capabilities or filesystem requirements.

## Licensing

FatFs is distributed under its upstream license.

Refer to the upstream repository and accompanying license
files for applicable licensing terms.
