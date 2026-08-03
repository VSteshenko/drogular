# Developer Tools example

This example demonstrates the complete Developer Tools extension path:

- `ApplicationProfile::Development` enables Developer Tools automatically.
- `SystemInfoContributor` adds an application-specific inspection section.
- The inspection JSON names the semantic renderer `example.system-info`.
- `system-info.js` is registered separately as the Diagnostics Page renderer.
- External IDEs and CLI clients can consume the same inspection JSON without using that renderer.

Run the example and open:

- Application: `http://localhost:8082/`
- Developer Tools: `http://localhost:8082/__drogular`
- Inspection JSON: `http://localhost:8082/__drogular/inspection`
