# syntax=docker/dockerfile:1
# check=skip=SecretsUsedInArgOrEnv
#
# The check is skipped because it flags ENV SSH_AUTH_SOCK by its name, but the
# value is only the agent socket path, no secret (keys never enter the image).
#
# RPV toolchain environment images, one per target (x86_64, microblaze, avr)
# plus 'all'. The toolchains are built by the same setup.sh as on a VM/PC.
#
# Build context is tools/env:
#   docker build -f tools/env/docker/Dockerfile --target x86_64 \
#       --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t rpv-env:x86_64 tools/env
#
# tools/env/docker/docker_shell.sh (Linux/WSL) and docker_shell.ps1 (Windows)
# build the image if needed and run it.

ARG DEBIAN_TAG=trixie-slim

# ---------------------------------------------------------------------------
# runtime: what every final image needs to run 'make <target>'
# ---------------------------------------------------------------------------
FROM debian:${DEBIAN_TAG} AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates cmake git make openssh-client libexpat1 libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

COPY docker/github_known_hosts /etc/ssh/ssh_known_hosts

# Same UID/GID as the host user, so files in the mounted repo stay theirs
ARG UID=1000
ARG GID=1000
RUN groupadd -o -g ${GID} rpv \
    && useradd -o -m -u ${UID} -g ${GID} -s /bin/bash rpv \
    && mkdir -p /opt/rpv /opt/rpv-env \
    && chown rpv:rpv /opt/rpv /opt/rpv-env

COPY --chown=rpv:rpv doctor.sh /opt/rpv-env/

# Starts an ssh-agent on SSH_AUTH_SOCK unless one is forwarded (docker_shell.sh)
COPY --chmod=755 docker/entrypoint.sh /usr/local/bin/rpv-entrypoint
ENTRYPOINT ["/usr/local/bin/rpv-entrypoint"]
CMD ["bash"]

ENV PATH=/opt/rpv/bin:$PATH
ENV SSH_AUTH_SOCK=/tmp/ssh-agent.sock

# ---------------------------------------------------------------------------
# builder: Buildroot host prerequisites, then one stage per toolchain
# ---------------------------------------------------------------------------
FROM runtime AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
        bc build-essential bzip2 cpio file patch perl rsync unzip wget xz-utils \
    && rm -rf /var/lib/apt/lists/*

USER rpv
COPY --chown=rpv:rpv . /opt/rpv-env/

FROM builder AS build-x86_64
RUN /opt/rpv-env/setup.sh --prefix /opt/rpv --clean --no-doctor --no-install x86_64

FROM builder AS build-microblaze
RUN /opt/rpv-env/setup.sh --prefix /opt/rpv --clean --no-doctor --no-install microblaze

FROM builder AS build-avr
RUN /opt/rpv-env/setup.sh --prefix /opt/rpv --clean --no-doctor --no-install avr

# ---------------------------------------------------------------------------
# final images: runtime + target tools + toolchain, checked by doctor.sh
# ---------------------------------------------------------------------------
FROM runtime AS x86_64

# ipxe-qemu: ROM for QEMU's default NIC, QEMU won't start without it
RUN apt-get update && apt-get install -y --no-install-recommends \
        qemu-system-x86 ipxe-qemu grub-common grub-pc-bin xorriso mtools fdisk dosfstools \
    && rm -rf /var/lib/apt/lists/*

USER rpv
COPY --from=build-x86_64 /opt/rpv/x86_64 /opt/rpv/x86_64
COPY --from=build-x86_64 /opt/rpv/bin /opt/rpv/bin
RUN /opt/rpv-env/doctor.sh --prefix /opt/rpv x86_64

FROM runtime AS microblaze

USER rpv
COPY --from=build-microblaze /opt/rpv/microblaze /opt/rpv/microblaze
COPY --from=build-microblaze /opt/rpv/bin /opt/rpv/bin
RUN /opt/rpv-env/doctor.sh --prefix /opt/rpv microblaze

FROM runtime AS avr

RUN apt-get update && apt-get install -y --no-install-recommends avrdude \
    && rm -rf /var/lib/apt/lists/*

USER rpv
COPY --from=build-avr /opt/rpv/avr /opt/rpv/avr
COPY --from=build-avr /opt/rpv/bin /opt/rpv/bin
RUN /opt/rpv-env/doctor.sh --prefix /opt/rpv avr

FROM runtime AS all

RUN apt-get update && apt-get install -y --no-install-recommends \
        qemu-system-x86 ipxe-qemu grub-common grub-pc-bin xorriso mtools fdisk dosfstools \
        avrdude \
    && rm -rf /var/lib/apt/lists/*

USER rpv
COPY --from=build-x86_64 /opt/rpv/x86_64 /opt/rpv/x86_64
COPY --from=build-microblaze /opt/rpv/microblaze /opt/rpv/microblaze
COPY --from=build-avr /opt/rpv/avr /opt/rpv/avr
COPY --from=build-x86_64 /opt/rpv/bin /opt/rpv/bin
COPY --from=build-microblaze /opt/rpv/bin /opt/rpv/bin
COPY --from=build-avr /opt/rpv/bin /opt/rpv/bin
RUN /opt/rpv-env/doctor.sh --prefix /opt/rpv all
