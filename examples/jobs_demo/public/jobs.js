const jobsNode = document.querySelector('#jobs');
const metricsNode = document.querySelector('#metrics');
const notice = document.querySelector('#notice');
// Live SSE is used for active Jobs; polling remains the recovery path.
const enableSse = true;
const connections = new Map();
const state = new Map();
// HTTP/1.1 browsers commonly allow only six connections per origin. Keep
// long-lived SSE streams bounded so API requests cannot starve behind them.
const maxLiveStreams = 2;
const retryAfter = new Map();
const reconnectDelayMs = 5000;
let refreshingJobs = false;
let refreshingMetrics = false;

const events = [
    'job.queued',
    'job.started',
    'job.progress',
    'job.cancellation-requested',
    'job.completed',
    'job.failed',
    'job.cancelled'
];
const terminal = new Set(['Completed', 'Failed', 'Cancelled']);
let lastNotice = '';

function showNotice(message) {
    if (message !== lastNotice) {
        notice.textContent = message;
        lastNotice = message;
    }
}

async function api(path, options = {}) {
    const response = await fetch(path, { cache: 'no-store', ...options });
    let body;
    try {
        body = await response.json();
    } catch {
        throw new Error(`HTTP ${response.status}`);
    }
    if (!response.ok) throw new Error(body.error || `HTTP ${response.status}`);
    return body;
}

function closeStream(id) {
    connections.get(id)?.close();
    connections.delete(id);
}

function updateConnectionStatus() {
    const element = document.querySelector('#connection');
    const active = [...state.values()].filter(job => !terminal.has(job.status)).length;
    element.textContent = active === 0 ? 'No active jobs'
        : !enableSse ? 'Polling · refresh every 2.5s'
        : connections.size ? `Live · SSE (${connections.size}/${active}); polling fallback`
        : 'Polling · SSE reconnect pending';
}

function reconcileStreams() {
    const now = Date.now();
    for (const id of connections.keys()) {
        if (terminal.has(state.get(id)?.status)) closeStream(id);
    }
    for (const job of state.values()) {
        if (connections.size >= maxLiveStreams) break;
        if (!terminal.has(job.status) && !connections.has(job.id) &&
            (retryAfter.get(job.id) || 0) <= now) subscribe(job);
    }
    updateConnectionStatus();
}

async function refreshOne(id) {
    try {
        const job = await api(`/api/jobs/detail?id=${encodeURIComponent(id)}`);
        state.set(job.id, job);
        if (terminal.has(job.status)) closeStream(job.id);
        reconcileStreams();
        render();
    } catch (error) { showNotice(error.message); }
}

function subscribe(job) {
    if (!enableSse) return;
    if (terminal.has(job.status) || connections.has(job.id) ||
        connections.size >= maxLiveStreams) return;
    const source = new EventSource(`/api/jobs/events?job=${encodeURIComponent(job.id)}`);
    connections.set(job.id, source);
    for (const eventName of events) {
        source.addEventListener(eventName, event => {
            const message = JSON.parse(event.data);
            const status = ({
                'job.queued':'Queued',
                'job.started':'Running',
                'job.progress':'Running',
                'job.completed':'Completed',
                'job.failed':'Failed',
                'job.cancelled':'Cancelled'
            })[eventName];
            const old = state.get(message.id) || job;
            state.set(message.id, {...old, ...message, status: status || old.status});
            render();
            if (status && terminal.has(status)) {
                closeStream(message.id);
                reconcileStreams();
                refreshOne(message.id);
            }
        });
    }
    source.addEventListener('job.reset', () => {
        closeStream(job.id);
        retryAfter.set(job.id, Date.now() + reconnectDelayMs);
        refreshOne(job.id);
    });
    source.onopen = () => {
        retryAfter.delete(job.id);
        updateConnectionStatus();
    };
    source.onerror = () => {
        // Do not allow half-open streams to monopolize the browser connection pool.
        // Polling continues, then a fresh subscription is established after backoff.
        closeStream(job.id);
        retryAfter.set(job.id, Date.now() + reconnectDelayMs);
        refreshOne(job.id);
        reconcileStreams();
    };
}

function render() {
    const jobs = [...state.values()].sort((a,b) => b.id - a.id);
    jobsNode.replaceChildren();
    if (!jobs.length) {
        const empty = document.createElement('p');
        empty.className = 'muted';
        empty.textContent = 'No jobs yet. Run a job to see live updates.';
        jobsNode.append(empty);
        return;
    }
    for (const job of jobs) {
        const row = document.createElement('article');
        row.className = 'job';
        const heading = document.createElement('div');
        heading.className = 'job-header';
        const name = document.createElement('strong');
        name.textContent = `#${job.id} · ${job.name}`;
        const status = document.createElement('span');
        status.className = `pill ${job.status.toLowerCase()}`;
        status.textContent = job.status;
        heading.append(name, status);
        const bar = document.createElement('progress');
        bar.max = 100;
        bar.value = job.progress;
        bar.setAttribute('aria-label', `${job.name} progress`);
        const detail = document.createElement('p');
        detail.className = 'muted';
        detail.textContent = `${job.progress}% · ${job.error || job.failure?.message || 
                                                    job.result || job.message || 'Waiting'}`;
        row.append(heading, bar, detail);
        if (!terminal.has(job.status)) {
            const button = document.createElement('button');
            button.className = 'secondary small';
            button.textContent = job.cancellationRequested ? 'Cancelling…' : 'Cancel';
            button.disabled = !!job.cancellationRequested;
            button.onclick = async () => {
                button.disabled = true;
                try {
                    state.set(job.id, await api(`/api/jobs/cancel?id=${job.id}`, {method:'POST'}));
                    render();
                } catch (error) { button.disabled = false; showNotice(error.message); }
            };
            row.append(button);
        }
        jobsNode.append(row);
    }
}

async function refreshJobs() {
    if (refreshingJobs) return;
    refreshingJobs = true;
    try {
        const jobs = await api('/api/jobs');
        for (const job of jobs) {
            state.set(job.id, job);
            if (terminal.has(job.status)) {
                closeStream(job.id);
                retryAfter.delete(job.id);
            }
        }
        reconcileStreams();
        render();
    } catch (error) { showNotice(error.message); }
    finally { refreshingJobs = false; }
}

async function refreshMetrics() {
    if (refreshingMetrics) return;
    refreshingMetrics = true;
    try {
        const metrics = await api('/api/jobs/diagnostics');
        metricsNode.replaceChildren();
        for (const [key, value] of Object.entries(metrics)) {
            const card = document.createElement('div');
            card.className = 'metric';
            const number = document.createElement('strong');
            number.textContent = Number(value).toLocaleString();
            const label = document.createElement('span');
            label.textContent = key.replace(/([A-Z])/g, ' $1');
            card.append(number, label);
            metricsNode.append(card);
        }
    } catch (error) { showNotice(error.message); }
    finally { refreshingMetrics = false; }
}

async function create(fail) {
    const duration = document.querySelector('#duration').value;
    try {
        const job = await api(`/api/jobs/create?seconds=${duration}&fail=${fail ? '1' : '0'}`, {method:'POST'});
        state.set(job.id, job);
        reconcileStreams();
        render();
        showNotice(`Job #${job.id} submitted`);
    } catch (error) { showNotice(error.message); }
}

document.querySelector('#launch').onclick = () => create(false);
document.querySelector('#fail').onclick = () => create(true);
for (const button of document.querySelectorAll('[data-tab]')) {
    button.onclick = () => {
        for (const tab of document.querySelectorAll('[data-tab]')) {
            tab.classList.toggle('selected', tab === button);
            if (tab === button) tab.setAttribute('aria-current', 'page');
            else tab.removeAttribute('aria-current');
        }
        document.querySelector('#jobs-panel').hidden = button.dataset.tab !== 'jobs';
        document.querySelector('#diagnostics-panel').hidden = button.dataset.tab !== 'diagnostics';
        if (button.dataset.tab === 'diagnostics') refreshMetrics();
    };
}

refreshJobs();
refreshMetrics();
setInterval(refreshJobs, 2500);
setInterval(refreshMetrics, 2000);
window.addEventListener('pagehide', () => {
    for (const id of connections.keys()) closeStream(id);
});