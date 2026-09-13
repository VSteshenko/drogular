# Drogular 0.22.0

Drogular 0.22 introduces a new project-generation workflow designed to make starting a Drogular application fast, predictable, and extensible.

The release adds a reusable project generator, an embedded template system, a developer-focused CLI, and two ready-to-build starter projects: a minimal Drogular application and a complete installable PWA.

## Highlights

### Project Generator

Drogular now includes a general-purpose project generation infrastructure.

The generator is separated from the command-line interface and is built around reusable components for:

- project template discovery;
- embedded template sources;
- template registration;
- variable substitution;
- directory and file generation;
- executable file handling;
- output path validation;
- cleanup after failed generation.

This separation keeps the CLI small while providing a foundation for future developer tooling and IDE integration.

### Drogular CLI

The `drogular` CLI can now create complete projects:

```bash
drogular new MyApp
```

Projects can be generated directly into a destination path:

```bash
drogular new examples/MyApp
```

A specific starter can be selected with:

```bash
drogular new MyPWA --template pwa
```

Available starters can be inspected with:

```bash
drogular templates
```

The CLI also provides improved help, version information, validation, and actionable next steps after successful generation.

### Embedded Project Templates

Official project templates are embedded directly into the Drogular CLI.

They do not depend on external template directories or additional downloads and are released together with the framework.

Generated projects are pinned to the Drogular release corresponding to the CLI that created them, making project creation reproducible across framework releases.

### Minimal Starter

`minimal` is the default project template:

```bash
drogular new MyApp
```

It provides a small but complete Drogular application with the recommended project structure, including:

- application startup;
- Page;
- reusable Component;
- templates;
- public assets;
- CMake configuration;
- starter README.

Short `Tip:` comments highlight useful extension points without turning generated source files into a tutorial.

The result is intended to be both a learning starting point and a practical base for a new application.

### PWA Starter

0.22 adds a ready-to-run Progressive Web Application starter:

```bash
drogular new MyPWA --template pwa
```

The generated application includes:

- PWA metadata;
- web app manifest;
- root-scoped service worker;
- application shell;
- offline fallback;
- application icons and favicon;
- responsive starter UI;
- reusable Drogular Pages and Components;
- build and usage documentation.

The generated project can be built and run immediately and is installable by supported browsers without requiring additional PWA setup.

### Safer Project Generation

Project generation now includes safeguards for common failure cases.

The generator:

- refuses to overwrite an existing destination;
- validates project names independently from destination paths;
- supports relative and absolute output paths;
- normalizes project names for generated C++ namespaces where required;
- prevents generated files from escaping the project root;
- removes partially generated projects when generation fails.

These guarantees make project generation suitable for use by higher-level tooling without requiring each caller to implement its own filesystem safety layer.

## Framework Improvements

Work on the project generator also exposed inconsistencies in framework startup and template handling.

### Template Root Consistency

Changing the application's template root now keeps template loading and `TemplateSourceCache` consistent.

This fixes cases where an application configured a custom template root but template lookup continued using an earlier location.

A regression test now protects this behavior.

### Reliable CLI Release Pinning

The Git reference embedded into generated projects is derived from the Drogular project version without persisting the computed value in the CMake cache.

This prevents an existing build directory from generating projects pinned to a stale Drogular release after the framework version changes.

## New Example: System Monitor PWA

0.22 also adds `system_monitor_pwa`, a more substantial example demonstrating how Drogular can be used to build a real-time installable monitoring application.

The example goes beyond a basic PWA shell and combines several Drogular capabilities in one application:

- live CPU, memory, disk, and process monitoring;
- streaming updates and connection-state handling;
- reconnect, retry, and offline behavior;
- persisted monitoring statistics;
- dedicated dashboard and hardware views;
- Linux and Docker support;
- remote host monitoring over SSH;
- Raspberry Pi detection and hardware information;
- Raspberry Pi health and thermal metrics;
- GPIO discovery and inspection;
- I²C, SPI, and UART interface information.

The example is developed and tested across macOS and Linux, with Raspberry Pi support providing a practical demonstration of remote hardware monitoring.

`system_monitor_pwa` also serves as a larger reference for Drogular's PWA architecture. Its UI uses Drogular Pages, Components, templates, and server-side rendering while keeping browser-side JavaScript small and focused on the behavior that genuinely belongs in the client.

In particular, the example demonstrates how live and offline-aware applications can progressively move presentation and state decisions into reusable C++ components instead of accumulating application-specific JavaScript.

Together, the two PWA additions in 0.22 serve different purposes:

- the `pwa` project template provides a compact starting point for a new application;
- `system_monitor_pwa` demonstrates how the same framework capabilities scale to a richer real-world application.

This makes the example useful not only as a demonstration, but also as a reference implementation for developers building monitoring dashboards, device-management interfaces, and other live Drogular applications.

## Testing

The project generator is covered at several levels:

- template source and registry tests;
- file renderer tests;
- project generation tests;
- path and failure handling tests;
- embedded starter tests;
- CLI parsing tests;
- template-root regression tests;
- generated-project end-to-end smoke tests.

The end-to-end tests generate both official starters, configure them against the current Drogular source tree, and build the resulting applications.

This verifies the complete path from:

```text
CLI
 → template discovery
 → project generation
 → CMake configuration
 → Drogular integration
 → application compilation and linking
```

Both `minimal` and `pwa` starters are therefore tested as real Drogular consumer projects rather than only as collections of generated files.

## Documentation

Getting Started and the root README have been updated for the new project workflow.

The documentation now covers:

- installing and using the Drogular CLI;
- creating projects;
- destination paths;
- listing available starters;
- selecting a project template;
- generated project structure;
- the `minimal` starter;
- the `pwa` starter;
- release pinning;
- testing generated applications against a local Drogular checkout.

## Looking Ahead

The project generator introduced in 0.22 is intentionally more general than the two starters included in this release.

Its architecture provides a foundation for future developer tooling such as additional project starters, generation of individual application artifacts, external template sources, and IDE integration without coupling those features directly to the CLI.

For 0.22, the focus remains deliberately narrow: provide a reliable and convenient way to go from an installed Drogular CLI to a working application with minimal setup.
