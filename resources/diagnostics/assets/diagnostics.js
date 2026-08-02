const endpoint = document.body.dataset.inspectionEndpoint || '/__drogular/inspection';

const byId = (id) => document.getElementById(id);
const escapeHtml = (value) => String(value ?? '')
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#039;');

const table = (columns, rows) => {
    if (!rows.length) return '<p class="empty">No entries.</p>';
    return `
<div class="table-wrap">
    <table>
        <thead>
            <tr>
                ${columns.map(c => `<th>${escapeHtml(c.label)}</th>`).join('')}
            </tr>
        </thead>
        <tbody>
            ${rows.map(row => `<tr>${columns.map(c => `<td>${c.render ? c.render(row) : escapeHtml(row[c.key])}</td>`).join('')}</tr>`).join('')}
        </tbody>
    </table>
</div>`;
};

const renderSummary = (data) => {
    const items = [
        ['Schema', data.schemaVersion],
        ['Routes', data.routes.length],
        ['Components', data.components.length],
        ['Services', data.services.length],
        ['Diagnostics', data.diagnostics.length]
    ];
    byId('summary').innerHTML = items.map(([label, value]) => `
<article class="summary-card">
    <span>
        ${escapeHtml(label)}
    </span>
    <strong>
        ${escapeHtml(value)}
    </strong>
</article>`).join('');
};

const showSection = (name, html) => {
    byId(`${name}-section`).hidden = false;
    byId(name).innerHTML = html;
};

const renderInspection = (data) => {
    renderSummary(data);
    showSection('routes', table([
        { label: 'Method', key: 'method', render: r => `<span class="badge">${escapeHtml(r.method)}</span>` },
        { label: 'Path', key: 'path', render: r => `<code>${escapeHtml(r.path)}</code>` },
        { label: 'Kind', key: 'kind' },
        { label: 'Target', key: 'target', render: r => `<code>${escapeHtml(r.target)}</code>` }
    ], data.routes));
    showSection('components', table([{ label: 'Tag', key: 'tag', render: r => `<code>${escapeHtml(r.tag)}</code>` }], data.components));
    showSection('services', table([
        { label: 'Type', key: 'type', render: r => `<code>${escapeHtml(r.type)}</code>` },
        { label: 'Lifetime', key: 'lifetime' },
        { label: 'Instantiated', key: 'instantiated', render: r => r.instantiated ? 'yes' : 'no' }
    ], data.services));

    const diagnostics = data.diagnostics.length
        ? data.diagnostics.map(d => `
<article class="diagnostic">
    <div class="diagnostic-header">
        <span class="badge">
            ${escapeHtml(d.severity)}
        </span>
        <code>
            ${escapeHtml(d.code)}
        </code>
    </div>
    <p class="diagnostic-message">
        ${escapeHtml(d.message)}
    </p>
    ${d.location
            ? `<p class="diagnostic-location">${escapeHtml(d.location.source || 'unknown')}:${escapeHtml(d.location.line)}:${escapeHtml(d.location.column)}</p>`
            : ''}
</article>`).join('')
        : '<p class="empty">No diagnostics reported.</p>';
    showSection('diagnostics', diagnostics);
};

const loadInspection = async () => {
    const button = byId('refresh-button');
    const error = byId('error-state');
    button.disabled = true;
    error.hidden = true;
    byId('status-text').textContent = 'Loading inspection data…';
    try {
        const controller = new AbortController();
        const timeout = window.setTimeout(() => controller.abort(), 10000);
        const response = await fetch(endpoint, {
            headers: { Accept: 'application/json' },
            cache: 'no-store',
            signal: controller.signal
        });
        window.clearTimeout(timeout);
        if (!response.ok) throw new Error(`Inspection request failed with HTTP ${response.status}`);
        const data = await response.json();
        renderInspection({
            schemaVersion: data.schemaVersion ?? 0,
            routes: Array.isArray(data.routes) ? data.routes : [],
            components: Array.isArray(data.components) ? data.components : [],
            services: Array.isArray(data.services) ? data.services : [],
            diagnostics: Array.isArray(data.diagnostics) ? data.diagnostics : []
        });
        byId('status-text').textContent = `Inspection loaded at ${new Date().toLocaleTimeString()}`;
    } catch (cause) {
        error.hidden = false;
        error.textContent = cause instanceof Error ? cause.message : String(cause);
        byId('status-text').textContent = 'Unable to load inspection data.';
    } finally {
        button.disabled = false;
    }
};

const startDiagnostics = () => {
    const refreshButton = byId('refresh-button');
    const statusText = byId('status-text');
    const errorState = byId('error-state');

    if (!refreshButton || !statusText || !errorState) {
        return;
    }

    refreshButton.addEventListener('click', loadInspection);
    loadInspection();
};

if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', startDiagnostics, { once: true });
} else {
    startDiagnostics();
}
