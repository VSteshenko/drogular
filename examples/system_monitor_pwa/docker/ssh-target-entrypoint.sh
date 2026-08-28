#!/usr/bin/env bash
set -euo pipefail

fixture_dir=${SSH_FIXTURE_DIR:-/ssh-fixture}

for file in client_ed25519.pub server_host_ed25519_key; do
    if [[ ! -f "${fixture_dir}/${file}" ]]; then
        echo "missing SSH fixture file: ${fixture_dir}/${file}" >&2
        exit 1
    fi
done

install -d -m 0700 -o monitor -g monitor /home/monitor/.ssh
install -m 0600 -o monitor -g monitor \
    "${fixture_dir}/client_ed25519.pub" \
    /home/monitor/.ssh/authorized_keys

mkdir -p /run/sshd
cat > /etc/ssh/sshd_config.d/system-monitor.conf <<CONFIG
PasswordAuthentication no
KbdInteractiveAuthentication no
PubkeyAuthentication yes
PermitRootLogin no
AllowUsers monitor
HostKey ${fixture_dir}/server_host_ed25519_key
PidFile /run/sshd.pid
CONFIG

exec /usr/sbin/sshd -D -e
