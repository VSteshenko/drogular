#!/usr/bin/env bash
set -euo pipefail

fixture_dir=${SSH_FIXTURE_DIR:-/ssh-fixture}
mkdir -p "${fixture_dir}"

if [[ ! -f "${fixture_dir}/client_ed25519" ]]; then
    ssh-keygen -q -t ed25519 -N '' -C 'system-monitor-client' \
        -f "${fixture_dir}/client_ed25519"
fi

if [[ ! -f "${fixture_dir}/server_host_ed25519_key" ]]; then
    ssh-keygen -q -t ed25519 -N '' -C 'ssh-target-host' \
        -f "${fixture_dir}/server_host_ed25519_key"
fi

chmod 0600 \
    "${fixture_dir}/client_ed25519" \
    "${fixture_dir}/server_host_ed25519_key"
chmod 0644 \
    "${fixture_dir}/client_ed25519.pub" \
    "${fixture_dir}/server_host_ed25519_key.pub"

printf 'ssh-target %s\n' "$(cat "${fixture_dir}/server_host_ed25519_key.pub")" \
    > "${fixture_dir}/known_hosts"
chmod 0644 "${fixture_dir}/known_hosts"
