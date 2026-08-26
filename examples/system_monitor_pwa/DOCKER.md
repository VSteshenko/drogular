# System Monitor PWA in Docker

The Docker setup runs the Linux version of `system_monitor_pwa`. By default,
the dashboard reports metrics for the container itself, not for the macOS host.

From the repository root:

```bash
docker compose -f examples/system_monitor_pwa/docker-compose.yml up --build
```

Open <http://localhost:8080>.

Stop the application with:

```bash
docker compose -f examples/system_monitor_pwa/docker-compose.yml down
```

The image uses a multi-stage build. Build dependencies remain in the builder
stage; the runtime stage contains the application, templates, static assets,
and the Drogon runtime library.

Host monitoring is intentionally not enabled at this stage. A later stage can
add explicit `/proc` and `/sys` mounts for Linux/Raspberry Pi hosts without
changing the normal container-monitoring mode.