#include <drogular/job_client_resources.hpp>

namespace drogular::job_client_resources {
namespace {

constexpr std::string_view Script = R"DROGULAR_JOB_JS(
/** Transport-only Jobs client. No DOM, UI state, or framework dependency. */
export class JobClient {
    constructor({baseUrl = '/api/jobs', fetchImpl = globalThis.fetch.bind(globalThis), eventSourceImpl = globalThis.EventSource} = {}) {
        this.baseUrl = baseUrl.replace(/\/$/, '');
        this.fetchImpl = fetchImpl;
        this.eventSourceImpl = eventSourceImpl;
    }

    async request(path, options = {}) {
        const response = await this.fetchImpl(`${this.baseUrl}${path}`, {
            cache: 'no-store', credentials: 'same-origin', ...options
        });
        let body;
        try { body = await response.json(); }
        catch { throw new Error(`HTTP ${response.status}: invalid JSON response`); }
        if (!response.ok) throw new Error(body?.error || `HTTP ${response.status}`);
        return body;
    }

    list() { return this.request(''); }
    get(id) { return this.request(`/detail?id=${encodeURIComponent(id)}`); }
    cancel(id) { return this.request(`/cancel?id=${encodeURIComponent(id)}`, {method: 'POST'}); }
    pause(id) { return this.request(`/pause?id=${encodeURIComponent(id)}`, {method: 'POST'}); }
    resume(id) { return this.request(`/resume?id=${encodeURIComponent(id)}`, {method: 'POST'}); }
    diagnostics() { return this.request('/diagnostics'); }
    /** Caller owns EventSource lifetime and MUST close it when done. */
    watch(id) {
        return new this.eventSourceImpl(`${this.baseUrl}/events?job=${encodeURIComponent(id)}`);
    }
})DROGULAR_JOB_JS";

} // namespace

std::string_view script() { return Script; }

} // namespace drogular::job_client_resources