
#!/usr/bin/env bash
set -eu

# The image entry point must already have started ssh-agent.
if [ -z "${SSH_AUTH_SOCK:-}" ]; then
    echo "Error: SSH_AUTH_SOCK is not set; ssh-agent may not be running." >&2
    exit 1
fi

if [ ! -d /mnt/ssh ]; then
    echo "Error: SSH directory is not mounted at /mnt/ssh." >&2
    exit 1
fi

mkdir -p "$HOME/.ssh"
chmod 700 "$HOME/.ssh"

# Copy recognized private keys only. Do not copy config or known_hosts.
for key in /mnt/ssh/*; do
    [ -f "$key" ] || continue

    case "$key" in
        *.pub|*/config|*/known_hosts|*/known_hosts.old|*/authorized_keys|*/environment)
            continue
            ;;
    esac

    if grep -qE '^-----BEGIN (OPENSSH|RSA|EC|DSA|ENCRYPTED) PRIVATE KEY-----' "$key"; then
        name=$(basename "$key")
        cp "$key" "$HOME/.ssh/$name"
        chmod 600 "$HOME/.ssh/$name"

        if ! ssh-add "$HOME/.ssh/$name"; then
            echo "Warning: could not load SSH key: $name" >&2
        fi
    fi
done

# Run the requested command, or start an interactive shell.
if [ "$#" -gt 0 ]; then
    exec "$@"
else
    exec bash -i
fi
