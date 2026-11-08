/** Transport-only Jobs client. No DOM, UI state, or framework dependency. */
export class JobClient {
    constructor({baseUrl = '/api/jobs', fetchImpl = fetch} = {}) {
        this.baseUrl = baseUrl.replace(/\/$/, '');
        this.fetchImpl = fetchImpl;
    }

    async request(path, options = {}) {
        const response = await this.fetchImpl(`${this.baseUrl}${path}`, {
            cache: 'no-store', credentials: 'same-origin', ...options
        });
        let body;
        try { body = await response.json(); }
        catch { throw new Error(`HTTP ${response.status}`); }
        if (!response.ok) throw new Error(body.error || `HTTP ${response.status}`);
        return body;
    }

    list() { return this.request(''); }
    get(id) { return this.request(`/detail?id=${encodeURIComponent(id)}`); }
    cancel(id) { return this.request(`/cancel?id=${encodeURIComponent(id)}`, {method: 'POST'}); }
    diagnostics() { return this.request('/diagnostics'); }
    // This endpoint is specific to JobsDemo, not a universal Registry contract.
    createDemo({seconds = 15, fail = false} = {}) {
        return this.request(`/create?seconds=${encodeURIComponent(seconds)}&fail=${fail ? '1' : '0'}`,
            {method: 'POST'});
    }
    /** Caller owns EventSource lifetime and MUST close it when done. */
    watch(id) {
        return new EventSource(`${this.baseUrl}/events?job=${encodeURIComponent(id)}`);
    }
}