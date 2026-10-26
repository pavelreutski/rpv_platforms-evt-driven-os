#!/usr/bin/env bash
#
# Run the RPV environment image of a target, building it first if needed.
# The repository is mounted at the same path as on the host, so 'make ...'
# works inside the container exactly as on a VM/PC.
#
#   tools/env/docker/docker_shell.sh x86_64                   # interactive shell
#   tools/env/docker/docker_shell.sh x86_64 make x86_64-iso   # single command
#   tools/env/docker/docker_shell.sh --rebuild x86_64         # rebuild the image first

set -euo pipefail

ENV_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
REPO="$(git -C "$ENV_DIR" rev-parse --show-toplevel)"

usage() {
    cat <<EOF
Usage: $(basename "$0") [--rebuild] <x86_64|microblaze|avr|all> [command...]

  --rebuild   rebuild the image even if it exists

Environment:
  RPV_AVR_PORT   host serial port, appears as /dev/ttyACM0 in avr/all containers
                 (default: /dev/ttyACM0)
EOF
}

REBUILD=0

case "${1:-}" in
    --rebuild) REBUILD=1; shift ;;
    -h|--help) usage; exit 0 ;;
esac

TARGET="${1:-}"
case "$TARGET" in
    x86_64|microblaze|avr|all) shift ;;
    *) usage >&2; exit 1 ;;
esac

IMAGE="rpv-env:$TARGET"

if [ "$REBUILD" -eq 1 ] || ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
    docker build -f "$ENV_DIR/docker/Dockerfile" --target "$TARGET" \
        --build-arg UID="$(id -u)" --build-arg GID="$(id -g)" \
        -t "$IMAGE" "$ENV_DIR"
fi

WORKDIR="$REPO"
case "$PWD/" in
    "$REPO"/*) WORKDIR="$PWD" ;;
esac

ARGS=(--rm -i -v "$REPO:$REPO" -w "$WORKDIR" -e TERM="${TERM:-xterm}")

if [ -t 0 ] && [ -t 1 ]; then
    ARGS+=(-t)
fi

# The kernel, guardian and FatFs are fetched over git@github.com. The
# agent socket is forwarded, the keys never enter the container.
if [ -n "${SSH_AUTH_SOCK:-}" ] && [ -S "$SSH_AUTH_SOCK" ]; then
    ARGS+=(-v "$SSH_AUTH_SOCK:/run/ssh-agent.sock" -e SSH_AUTH_SOCK=/run/ssh-agent.sock)
else
    echo "note: no host ssh-agent to forward, the container starts its own: ssh-add <key> there" >&2
fi

if [ "$TARGET" = "avr" ] || [ "$TARGET" = "all" ]; then
    PORT="${RPV_AVR_PORT:-/dev/ttyACM0}"

    if [ -e "$PORT" ]; then
        ARGS+=(--device "$PORT:/dev/ttyACM0" --group-add "$(stat -c %g "$PORT")")
    fi
fi

if [ $# -eq 0 ]; then
    set -- bash
fi

exec docker run "${ARGS[@]}" "$IMAGE" "$@"
