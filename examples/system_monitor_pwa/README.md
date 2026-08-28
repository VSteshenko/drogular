# System Monitor PWA

`system_monitor_pwa` is a Drogular example that monitors either the local
macOS/Linux host or a remote Linux host, including Raspberry Pi, over SSH.

The remote mode uses public-key authentication and strict `known_hosts`
verification. Password authentication is not used by the application.

## Local monitoring

Local monitoring is the default. `SYSTEM_MONITOR_TARGET` may be omitted or set
explicitly:

```bash
export SYSTEM_MONITOR_TARGET=local
./build/examples/system_monitor_pwa/system_monitor_pwa
```

On macOS this uses the native macOS metrics provider. On Linux it reads local
Linux system information.

## Raspberry Pi over SSH

The monitor can run on macOS or Linux while the Raspberry Pi remains a normal
Linux SSH host. Drogular does not need to be installed on the Pi.

### 1. Prepare Raspberry Pi

Make sure the SSH server is enabled:

```bash
sudo systemctl enable --now ssh
sudo systemctl status ssh
```

Create a dedicated unprivileged account:

```bash
sudo adduser monitor
```

Do not add `monitor` to `sudo`. The current system metrics only require
unprivileged access.

Verify that the account can read the data used by the Linux provider:

```bash
su - monitor
cat /proc/meminfo | head
cat /proc/stat | head
cat /proc/loadavg
cat /proc/uptime
df -P -B1 /
hostname
uname -s
uname -r
uname -m
```

### 2. Create a host-side SSH key

On the machine running System Monitor:

```bash
ssh-keygen -t ed25519 \
    -f ~/.ssh/system_monitor_pi \
    -C "drogular-system-monitor"
```

For unattended monitoring, the key must be usable without an interactive
passphrase prompt. Keep the private key readable only by its owner:

```bash
chmod 600 ~/.ssh/system_monitor_pi
```

Install the public key for the `monitor` account. If `ssh-copy-id` is available:

```bash
ssh-copy-id -i ~/.ssh/system_monitor_pi.pub monitor@<PI_HOST>
```

Otherwise:

```bash
cat ~/.ssh/system_monitor_pi.pub | \
ssh monitor@<PI_HOST> \
'mkdir -p ~/.ssh && chmod 700 ~/.ssh && cat >> ~/.ssh/authorized_keys && chmod 600 ~/.ssh/authorized_keys'
```

Verify public-key login:

```bash
ssh -i ~/.ssh/system_monitor_pi monitor@<PI_HOST> hostname
```

### 3. Create a dedicated known_hosts file

System Monitor deliberately refuses unknown or changed host keys. Create a
dedicated file on the host:

```bash
ssh-keyscan -H <PI_HOST> > ~/.ssh/system_monitor_pi_known_hosts
chmod 600 ~/.ssh/system_monitor_pi_known_hosts
```

Before trusting the scanned key, compare its fingerprint with the Raspberry
Pi host key fingerprint obtained directly on the Pi, for example:

```bash
sudo ssh-keygen -lf /etc/ssh/ssh_host_ed25519_key.pub
```

Then verify the complete SSH configuration from the host:

```bash
ssh \
    -i ~/.ssh/system_monitor_pi \
    -o UserKnownHostsFile="$HOME/.ssh/system_monitor_pi_known_hosts" \
    -o StrictHostKeyChecking=yes \
    monitor@<PI_HOST> hostname
```

### 4. Configure System Monitor

Remote monitoring requires a build with libssh support.

Set the environment before starting the application:

```bash
export SYSTEM_MONITOR_TARGET=ssh
export SYSTEM_MONITOR_SSH_HOST=<PI_HOST>
export SYSTEM_MONITOR_SSH_PORT=22
export SYSTEM_MONITOR_SSH_USER=monitor
export SYSTEM_MONITOR_SSH_IDENTITY_FILE="$HOME/.ssh/system_monitor_pi"
export SYSTEM_MONITOR_SSH_KNOWN_HOSTS_FILE="$HOME/.ssh/system_monitor_pi_known_hosts"

./build/examples/system_monitor_pwa/system_monitor_pwa
```

Open <http://localhost:8080>. The dashboard should show the Raspberry Pi
hostname and Linux metrics rather than metrics from the machine running the
application.

## Environment variables

| Variable | Required | Default | Description |
| --- | --- | --- | --- |
| `SYSTEM_MONITOR_TARGET` | No | `local` | `local` or `ssh` |
| `SYSTEM_MONITOR_SSH_HOST` | SSH only | — | Remote hostname or IP address |
| `SYSTEM_MONITOR_SSH_PORT` | No | `22` | SSH port, 1-65535 |
| `SYSTEM_MONITOR_SSH_USER` | SSH only | — | Remote SSH account |
| `SYSTEM_MONITOR_SSH_IDENTITY_FILE` | SSH only | — | Path to the private key |
| `SYSTEM_MONITOR_SSH_KNOWN_HOSTS_FILE` | SSH only | — | Path to the strict `known_hosts` file |

All four SSH-specific path/host/user variables are required when
`SYSTEM_MONITOR_TARGET=ssh`. If the application was built without libssh,
requesting the SSH target fails at startup.

## Connection recovery

If an established SSH connection is lost, System Monitor keeps the last
successful snapshot and marks it as `Stale`. Subsequent refreshes attempt to
reconnect. When the Raspberry Pi becomes reachable again, a fresh snapshot is
collected and the dashboard automatically returns to `Live`.

An authentication failure or rejected host key is treated as a configuration
or security error and must be corrected rather than bypassed.
