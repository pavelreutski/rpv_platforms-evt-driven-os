#!/bin/sh
#
# Image entrypoint: make sure an ssh-agent is available, then run the command.
#
# shell.sh forwards the host agent (SSH_AUTH_SOCK=/run/ssh-agent.sock). Without
# it, an agent is started on the image's default socket, so 'ssh-add <key>'
# works in this container and in 'docker exec' shells. Adding keys is up to
# the user.

# ssh-add -l exit status: 0 keys loaded, 1 no keys, 2 no agent reachable
ssh-add -l >/dev/null 2>&1

if [ $? -eq 2 ]; then
    rm -f "$SSH_AUTH_SOCK"
    ssh-agent -a "$SSH_AUTH_SOCK" >/dev/null
fi

exec "$@"
