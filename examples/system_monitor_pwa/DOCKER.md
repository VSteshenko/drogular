# System Monitor PWA in Docker

The normal Docker setup runs the Linux version of `system_monitor_pwa` and
reports metrics for the application container itself, not for the macOS host.

From the repository root:

```bash
docker compose -f examples/system_monitor_pwa/docker-compose.yml up --build
```

Open <http://localhost:8080>.

Stop the application with:

```bash
docker compose -f examples/system_monitor_pwa/docker-compose.yml down
```

## SSH integration test

The `ssh-test` profile adds a second Linux container and monitors that container
through the real `SshSystemReader -> LibsshClient -> SSH` path.

The test setup does not store private keys in the repository. An ephemeral
`ssh-fixture` service creates:

- an Ed25519 client key used by `system-monitor-ssh`;
- an Ed25519 host key used by `ssh-target`;
- a matching strict `known_hosts` file.

Start the complete test environment:

```bash
docker compose -f examples/system_monitor_pwa/docker-compose.yml \
    --profile ssh-test up --build
```

The dashboards are then available at:

- <http://localhost:8080> — local container monitoring;
- <http://localhost:8081> — `ssh-target` monitoring through SSH.

The remote dashboard should report `ssh-target` as its hostname.

To run only the remote-monitoring path and its dependencies:

```bash
docker compose -f examples/system_monitor_pwa/docker-compose.yml \
    --profile ssh-test up --build system-monitor-ssh
```

Remove the containers and generated SSH fixture volume with:

```bash
docker compose -f examples/system_monitor_pwa/docker-compose.yml \
    --profile ssh-test down -v
```

The image uses a multi-stage build. Build dependencies remain in the builder
stage; the runtime stage contains the application, templates, static assets,
and required runtime libraries.

Host monitoring is intentionally not enabled at this stage. A later stage can
add explicit `/proc` and `/sys` mounts for Linux/Raspberry Pi hosts without
changing the normal container-monitoring mode.
