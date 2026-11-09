# Job Registry (Drogular 0.26)

`JobRegistry` is a transport-neutral catalog of explicitly registered Job types.
It owns a non-owning reference to a `JobRuntime`, and applications must ensure
that the runtime outlives the registry. `JobRuntime::submit()` remains available
for jobs that do not need catalog-based discovery.

Each `JobDefinition` contains a stable identifier, display name, description,
cancellation metadata and typed parameter definitions (integer, boolean, string).
The registry validates parameter names, required arguments, default values,
integer ranges and boolean literals *before* submitting work. Each launch
captures the factory and validated parameters by value.

`JobRegistryContributor` publishes a read-only `job-registry` inspection section
using the built-in `drogular.job-registry` renderer. A launch endpoint is optional
and **never automatically registered**. The developer-tools renderer only shows
launch controls when the contributor is given an explicit endpoint URL.

Applications must authenticate users, authorize each job type, validate CSRF
and request origins, and scope job results and event access to the requesting
principal. Registration and discovery alone never imply authorization.

JobsDemo registers `demo.batch` and `demo.failure`, and opts into a local-only
launch route. The example is not a production security reference.

The opt-in `App::jobClient()` serves the DOM-free `JobClient` ES module at
`/__drogular/assets/job-client.js`. It handles basic HTTP and job-scoped SSE
transport only; visual state and retry policies belong to the host application.
Registration of the module does not expose server-side job routes or permissions.