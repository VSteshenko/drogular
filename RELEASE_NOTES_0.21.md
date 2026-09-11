# Drogular 0.21

Drogular 0.21 focuses on developer experience, application diagnostics, and production readiness. It adds a standalone project generator, turns Developer Tools into an extensible platform, introduces structured application inspection, and strengthens dependency-injection behavior under concurrent requests.

## Highlights

### Drogular CLI and project generator

Drogular now includes a standalone CLI for bootstrapping applications:

```bash
drogular new hello_drogular
```

Generated projects include the recommended CMake setup, Page and Component structure, templates, public assets, and startup code. Official builds pin generated projects to the matching Drogular release tag for reproducible builds. Development builds can override the generated Git reference with `DROGULAR_CLI_GIT_REF`.

### Developer Tools platform

Developer Tools are now built on public framework APIs rather than being a fixed diagnostics page. The platform includes:

- Development-profile integration
- built-in Diagnostics Page
- structured Application Inspection
- JSON inspection endpoint
- extensible inspection contributors
- custom developer-tool components
- embedded frontend resources

Applications and libraries can contribute their own diagnostics without modifying Drogular itself.

### Application Inspection

The new inspection model separates runtime inspection data from its presentation. The same structured data can be consumed by the built-in Diagnostics Page, the JSON endpoint, or external development tools.

PortalDemo demonstrates application-specific inspection contributors and custom diagnostics sections.

### Dependency injection and thread safety

0.21 closes several concurrency and lifetime gaps:

- `TemplateSourceCache` is thread-safe for application-wide use
- first resolution of lazy singletons is synchronized
- scoped services are shared across the request context
- `ActionContext` participates in request-scoped resolution
- child `RenderContext` instances inherit the same request scope
- independent root/request contexts receive independent scoped instances

The corresponding behavior is covered by multithreaded and lifetime tests.

## Documentation

The documentation received a broad revision and expansion, including:

- improved Getting Started guidance
- project creation with the Drogular CLI
- expanded API Reference
- Dependency Injection
- Routing
- Authentication & Sessions
- Pagination & Request Parameters
- PWA
- Static Files
- Developer Tools
- Testing
- GraphQL and Cookbook material

The documentation now describes recommended application structure and framework architecture more consistently.

## Examples and reference application

PortalDemo continues to serve as the reference application and now demonstrates extensible application diagnostics through public inspection APIs.

The release also includes a dedicated Developer Tools example alongside focused examples such as TodoPWA, Auth Sample, and Repository Sample.

## Testing

0.21 expands coverage around:

- Developer Tools and inspection contributors
- request-scoped dependency injection
- lazy singleton concurrency
- template source caching
- CLI template expansion
- generated project structure and release pinning

## Compatibility

There are no intentional breaking API changes in this release. Existing 0.20 applications should be able to upgrade without mandatory migration work.

New 0.21 facilities such as the CLI, Developer Tools contributors, Application Inspection, and request-scoped services are opt-in where applicable.

## Looking ahead: 0.22

Drogular 0.22 will focus on **Developer Productivity**. Planned areas include CLI generators, feature-first scaffolding, project templates, feature/CRUD generation, an asset pipeline, Localization 2.0, further PortalDemo expansion, and additional architecture and best-practices documentation.
