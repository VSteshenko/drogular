# Browser JobClient (Drogular 0.26)

`App::jobClient()` is opt-in and serves an ES module at
`/__drogular/assets/job-client.js`. No script files need to be copied into
an application's static assets. The module is independent of the DOM and the
server-side Job Runtime; using it does **not** register any HTTP endpoints.

```javascript
import {JobClient} from '/__drogular/assets/job-client.js';
const jobs = new JobClient({baseUrl: '/api/jobs'});
const snapshots = await jobs.list();
const one = await jobs.get(123);
await jobs.pause(123);
await jobs.resume(123);
await jobs.cancel(123);
const diagnostics = await jobs.diagnostics();
const stream = jobs.watch(123);
stream.addEventListener('job.progress', event => {
    const snapshot = JSON.parse(event.data);
    console.log(snapshot.progress);
});
// Applications must close streams on terminal events, navigation and disposal.
stream.close();
```

The default transport contract uses `GET <baseUrl>`,
`GET <baseUrl>/detail?id=...`, `POST <baseUrl>/pause?id=...`,
`POST <baseUrl>/resume?id=...`, `POST <baseUrl>/cancel?id=...`,
`GET <baseUrl>/diagnostics` and `GET <baseUrl>/events?job=...`.
Applications control authentication, authorization, CSRF defenses and endpoint
publication. The client uses `credentials: 'same-origin'` and `cache: 'no-store'`
for fetch requests. It accepts `fetchImpl` and `eventSourceImpl` for testing.
The returned native EventSource has its standard lifecycle: callers manage
reconnection, `job.reset`, stream limits and terminal cleanup. The module does
not automatically create jobs because every application's launch contract and
authorization requirements differ. JobsDemo's `/create` route is demo-only.
