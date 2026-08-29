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

Raspberry Pi health monitoring also uses `vcgencmd get_throttled`. A dedicated
unprivileged account may not have access to the firmware device by default. If

```bash
su - monitor
vcgencmd get_throttled
```

fails with an error such as:

```text
Can't open device file: /dev/vcio_gencmd
```

grant the `monitor` account access through the `video` group. First make sure
the group exists:

```bash
getent group video
sudo usermod -aG video monitor
```

Do not add `monitor` to `sudo`. Grant access only to the firmware device nodes.
To make the device permissions persistent across reboots, create
`/etc/udev/rules.d/99-vcio.rules`:

```text
KERNEL=="vcio", GROUP="video", MODE="0660"
KERNEL=="vcio_gencmd", GROUP="video", MODE="0660"
```

Reload the udev rules and apply them:

```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Start a new login or SSH session for `monitor` so that its new group membership
takes effect, then verify the setup:

```bash
groups
ls -l /dev/vcio /dev/vcio_gencmd
vcgencmd get_throttled
```

A healthy Raspberry Pi normally reports:

```text
throttled=0x0
```

If `vcgencmd` is unavailable or the firmware status cannot be read, System
Monitor keeps Raspberry Pi health optional: the rest of the snapshot remains
available and the API reports `raspberryPi.health` as `null`.

CPU frequency is read independently from Linux cpufreq and does not require
firmware access:

```bash
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq
```

The value is reported in kHz by Linux and converted to Hz by System Monitor.

GPIO discovery uses the Linux GPIO character devices through the libgpiod
command-line tools. On Raspberry Pi OS these devices are normally owned by the
`gpio` group:

```bash
ls -l /dev/gpiochip*
```

For a dedicated `monitor` account, add it to that group:

```bash
sudo usermod -aG gpio monitor
```

Start a new login or SSH session so that the new group membership takes effect,
then verify access:

```bash
groups
gpiodetect
```

`groups` should include `gpio`, and `gpiodetect` should list the available GPIO
chips instead of reporting `Permission denied`. For example, a Raspberry Pi 4
may expose `gpiochip0` and `gpiochip1`.

System Monitor uses this modern gpiochip/libgpiod interface rather than the
deprecated sysfs GPIO interface. GPIO monitoring is initially read-only and
does not require adding `monitor` to `sudo`.


I²C inventory uses the `i2c-tools` command-line utilities. Install them on the
Linux target:

```bash
sudo apt update
sudo apt install i2c-tools
```

The I²C device nodes are normally owned by the `i2c` group:

```bash
ls -l /dev/i2c-*
```

Add the dedicated monitoring account to that group:

```bash
sudo usermod -aG i2c monitor
```

Start a new login or SSH session so that the new group membership takes effect,
then verify access:

```bash
groups
i2cdetect -l
```

`groups` should include `i2c`. To scan a specific user-facing bus manually, for
example the standard Raspberry Pi GPIO2/GPIO3 I²C bus:

```bash
i2cdetect -y 1
```

A device at address `0x3c` appears as `3c` in the scan table.

System Monitor always inventories adapters with `i2cdetect -l`, but address
scanning is opt-in because `i2cdetect` actively probes devices and should not
be run automatically on unknown or internal buses. Select only buses that are
safe to probe:

```bash
export SYSTEM_MONITOR_I2C_SCAN_BUSES=1
```

Multiple buses may be supplied as a comma-separated list:

```bash
export SYSTEM_MONITOR_I2C_SCAN_BUSES=1,4
```

The I²C snapshot is cached for five minutes, so repeated dashboard or API
requests do not continuously probe the bus. Buses not listed in
`SYSTEM_MONITOR_I2C_SCAN_BUSES` remain visible in the inventory with
`scanned: false`, but their addresses are not probed.

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
export SYSTEM_MONITOR_I2C_SCAN_BUSES=1

./build/examples/system_monitor_pwa/system_monitor_pwa
```

Open <http://localhost:8080>. The dashboard should show the Raspberry Pi
hostname and Linux metrics rather than metrics from the machine running the
application.

### 5. Verify GPIO and I²C APIs

From the machine running System Monitor:

```bash
curl -s http://localhost:8080/api/gpio
curl -s http://localhost:8080/api/i2c
```

`/api/i2c` does not require separate HTTP authentication in this example. Its
ability to enumerate and scan the remote buses depends on the permissions of
the SSH account (`monitor`) and on `i2c-tools` being installed on the target.

With `SYSTEM_MONITOR_I2C_SCAN_BUSES=1` and a responding device at `0x3c`, the
I²C response contains data similar to:

```json
{
  "available": true,
  "buses": [
    {
      "number": 1,
      "name": "i2c-1",
      "scanned": true,
      "gpioPins": [
        {
          "role": "sda",
          "chip": "gpiochip0",
          "offset": 2,
          "name": "GPIO2",
          "function": "SDA1"
        },
        {
          "role": "scl",
          "chip": "gpiochip0",
          "offset": 3,
          "name": "GPIO3",
          "function": "SCL1"
        }
      ],
      "devices": [
        {
          "address": 60,
          "addressHex": "0x3c",
          "claimedByKernel": false
        }
      ]
    }
  ]
}
```

When GPIO pin-mux information is available, `/api/i2c` also correlates a bus
with matching `SDA<N>` and `SCL<N>` functions from `/api/gpio`. For example,
Raspberry Pi bus `i2c-1` is enriched with GPIO2 / `SDA1` and GPIO3 / `SCL1`.
This is optional enrichment: I²C inventory remains available when GPIO or
`pinctrl` information is unavailable.

If a bus is present but not selected for scanning, it is still returned with
`"scanned": false` and an empty `devices` array. If `i2cdetect` reports
`Permission denied`, verify that the remote `monitor` account belongs to the
`i2c` group and reconnect the SSH session.

## Environment variables

| Variable | Required | Default | Description |
| --- | --- | --- | --- |
| `SYSTEM_MONITOR_TARGET` | No | `local` | `local` or `ssh` |
| `SYSTEM_MONITOR_SSH_HOST` | SSH only | — | Remote hostname or IP address |
| `SYSTEM_MONITOR_SSH_PORT` | No | `22` | SSH port, 1-65535 |
| `SYSTEM_MONITOR_SSH_USER` | SSH only | — | Remote SSH account |
| `SYSTEM_MONITOR_SSH_IDENTITY_FILE` | SSH only | — | Path to the private key |
| `SYSTEM_MONITOR_SSH_KNOWN_HOSTS_FILE` | SSH only | — | Path to the strict `known_hosts` file |
| `SYSTEM_MONITOR_I2C_SCAN_BUSES` | No | — | Comma-separated I²C bus numbers allowed for active address scanning |

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
