
# lwIP

Vendored Lightweight IP (lwIP) networking stack.

## Upstream

- Project: lwIP
- Repository Mirror: https://github.com/lwip-tcpip/lwip

## Directory structure

- `upstream/source/` — Upstream lwIP source code.
- `bridge/` — Specific integration between the kernel
  networking subsystem and lwIP.
- `config/lwipopts.h.in` — Specific lwIP configuration template.

## Integration

lwIP is integrated through the project CMake
build system.

The `bridge/` component provides the integration boundary
between lwIP and kernel services, including the
network interface and operating-system abstraction.

The configuration template is used to generate `lwipopts.h`
in the build output directory.

## Source ownership

Upstream lwIP sources are vendored and maintained locally
within this repository.

Integration code and configuration are
maintained separately from upstream sources.

Upstream updates must be explicitly reviewed and integrated.
The upstream source snapshot should remain unmodified
whenever possible.

## Configuration policy

The current configuration is shared across targets.

Target-specific configurations may be introduced when
required by platform capabilities or networking requirements.

## Licensing

lwIP is distributed under its upstream license.

Refer to the upstream repository and accompanying license
files for applicable licensing terms.
