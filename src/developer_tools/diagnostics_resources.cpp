#include <drogular/developer_tools/diagnostics_resources.hpp>

namespace drogular::diagnostics_resources {

namespace {

constexpr std::string_view PageHtml = R"DROGULAR_HTML(<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Drogular Diagnostics</title>
    <link rel="stylesheet" href="/__drogular/assets/diagnostics.css">
</head>
<body data-inspection-endpoint="/__drogular/inspection" data-components-endpoint="/__drogular/developer-tools/components">
    <main id="diagnostics-app" class="diagnostics-shell" aria-live="polite">
        <header class="diagnostics-header">
            <div>
                <p class="eyebrow">Drogular Developer Tools</p>
                <h1>Application Diagnostics</h1>
                <p id="status-text" class="status-text">Loading inspection data…</p>
            </div>
            <button id="refresh-button" type="button">Refresh</button>
        </header>
        <section id="summary" class="summary-grid" aria-label="Application summary"></section>
        <section id="error-state" class="state-panel" hidden></section>
        <section id="routes-section" class="panel" hidden>
            <h2>Routes</h2><div id="routes"></div>
        </section>
        <section id="components-section" class="panel" hidden>
            <h2>Components</h2><div id="components"></div>
        </section>
        <section id="services-section" class="panel" hidden>
            <h2>Services</h2><div id="services"></div>
        </section>
        <section id="diagnostics-section" class="panel" hidden>
            <h2>Diagnostics</h2><div id="diagnostics"></div>
        </section>
        <div id="extension-sections"></div>
    </main>
<script defer src="/__drogular/assets/diagnostics.js"></script>
</body>
</html>)DROGULAR_HTML";

constexpr std::string_view Stylesheet = R"DROGULAR_CSS(:root {
    color-scheme: light dark;
    font-family:
            Inter,
            ui-sans-serif,
            system-ui,
            -apple-system,
            BlinkMacSystemFont,
            "Segoe UI",
            sans-serif;
    background: #0b1020;
    color: #e8ecf5;
}

* {
    box-sizing: border-box;
}

body {
    margin: 0;
    min-height: 100vh;
    background: radial-gradient(circle at top, #18213d, #0b1020 48%);
}

button {
    font: inherit;
}

.diagnostics-shell {
    width: min(1180px, calc(100% - 32px));
    margin: 0 auto;
    padding: 48px 0 72px;
}

.diagnostics-header {
    display: flex;
    justify-content: space-between;
    gap: 24px;
    align-items: flex-start;
    margin-bottom: 28px;
}

.eyebrow {
    margin: 0 0 8px;
    color: #89a7ff;
    font-size: .78rem;
    font-weight: 700;
    letter-spacing: .12em;
    text-transform: uppercase;
}

h1 {
    margin: 0;
    font-size: clamp(2rem, 5vw, 3.5rem);
    letter-spacing: -.04em;
}

h2 {
    margin: 0 0 18px;
    font-size: 1.05rem;
}

.status-text {
    margin: 10px 0 0;
    color: #aab4cc;
}

#refresh-button {
    border: 1px solid #5069b7;
    border-radius: 10px;
    padding: 10px 16px;
    background: #21346f;
    color: white;
    cursor: pointer;
}

#refresh-button:disabled {
    cursor: progress;
    opacity: .65;
}

.summary-grid {
    display: grid;
    grid-template-columns: repeat(5, minmax(0, 1fr));
    gap: 12px;
    margin-bottom: 20px;
}

.summary-card, .panel, .state-panel {
    border: 1px solid rgba(151, 166, 205, .18);
    background: rgba(17, 24, 46, .86);
    box-shadow: 0 16px 48px rgba(0,0,0,.18);
}

.summary-card {
    border-radius: 14px;
    padding: 18px;
}

.summary-card span {
    display: block;
    color: #97a3bd;
    font-size: .78rem;
    text-transform: uppercase;
    letter-spacing: .08em;
}

.summary-card strong {
    display: block;
    margin-top: 8px;
    font-size: 1.65rem;
}

.panel, .state-panel {
    border-radius: 16px;
    padding: 22px;
    margin-top: 16px;
    overflow: hidden;
}

.state-panel {
    border-color: rgba(255, 111, 111, .45);
    color: #ffd0d0;
}

.table-wrap {
    overflow-x: auto;
}

table {
    width: 100%;
    border-collapse: collapse;
}

th, td {
    padding: 12px 10px;
    border-bottom: 1px solid rgba(151,166,205,.13);
    text-align: left;
    vertical-align: top;
}

th {
    color: #97a3bd;
    font-size: .75rem;
    letter-spacing: .08em;
    text-transform: uppercase;
}

td {
    font-size: .92rem;
}

code, .badge {
    font-family:
            ui-monospace,
            SFMono-Regular,
            Menlo,
            monospace;
}

.badge {
    display: inline-block;
    border-radius: 999px;
    padding: 3px 8px;
    background: rgba(104, 133, 255, .14);
    color: #b9c8ff;
    font-size: .76rem;
}

.empty {
    margin: 0;
    color: #8f9ab2;
}

.diagnostic {
    padding: 14px 0;
    border-bottom: 1px solid rgba(151,166,205,.13);
}

.diagnostic:last-child {
    border-bottom: 0;
}

.diagnostic-header {
    display: flex;
    gap: 10px;
    align-items: center;
    margin-bottom: 7px;
}

.diagnostic-message {
    margin: 0;
}

.diagnostic-location {
    margin: 6px 0 0;
    color: #8f9ab2;
    font-size: .82rem;
}

@media (max-width: 820px) {
    .summary-grid {
        grid-template-columns: repeat(2, 1fr);
    }
    .diagnostics-header {
        flex-direction: column;
    }
}

@media (max-width: 480px) {
    .summary-grid {
        grid-template-columns: 1fr;
    }
    .diagnostics-shell {
        width: min(100% - 20px, 1180px);
        padding-top: 28px;
    }
}

.json-tree {
    margin: 0;
    white-space: pre-wrap;
    overflow-wrap: anywhere;
    font-size: .84rem;
    line-height: 1.55;
    color: #c8d2ea;
}

.component-note {
    margin: 0 0 12px;
    color: #f7c873;
}
)DROGULAR_CSS";

constexpr std::string_view Script = R"DROGULAR_JS(const inspectionEndpoint = document.body.dataset.inspectionEndpoint || '/__drogular/inspection';
const componentsEndpoint = document.body.dataset.componentsEndpoint || '/__drogular/developer-tools/components';

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
            <tr>${columns.map(c => `
                <th>${escapeHtml(c.label)}</th>`).join('')}
            </tr>
        </thead>
        <tbody>${rows.map(row => `
            <tr>${columns.map(c => `
                <td>
                    ${c.render ? c.render(row) : escapeHtml(row[c.key])}
                </td>`).join('')}
            </tr>`).join('')}
        </tbody>
    </table>
</div>`;
};

const builtInRenderers = new Map([
    ['drogular.routes', (container, section) => {
        container.innerHTML = table([
            { label: 'Method', key: 'method', render: r => `<span class="badge">${escapeHtml(r.method)}</span>` },
            { label: 'Path', key: 'path', render: r => `<code>${escapeHtml(r.path)}</code>` },
            { label: 'Kind', key: 'kind' },
            { label: 'Target', key: 'target', render: r => `<code>${escapeHtml(r.target)}</code>` }
        ], Array.isArray(section.data) ? section.data : []);
    }],
    ['drogular.components', (container, section) => {
        container.innerHTML = table([
            { label: 'Tag', key: 'tag', render: r => `<code>${escapeHtml(r.tag)}</code>` }
        ], Array.isArray(section.data) ? section.data : []);
    }],
    ['drogular.services', (container, section) => {
        container.innerHTML = table([
            { label: 'Type', key: 'type', render: r => `<code>${escapeHtml(r.type)}</code>` },
            { label: 'Lifetime', key: 'lifetime' },
            { label: 'Instantiated', key: 'instantiated', render: r => r.instantiated ? 'yes' : 'no' }
        ], Array.isArray(section.data) ? section.data : []);
    }],
    ['drogular.diagnostics', (container, section) => {
        const diagnostics = Array.isArray(section.data) ? section.data : [];
        container.innerHTML = diagnostics.length
            ? diagnostics.map(d => `
<article class="diagnostic">
    <div class="diagnostic-header">
        <span class="badge">
            ${escapeHtml(d.severity)}
        </span>
        <code>
            ${escapeHtml(d.code)}
        </code>
    </div>
    <p class="diagnostic-message">${escapeHtml(d.message)}</p>
    ${d.location ? `
<p class="diagnostic-location">
    ${escapeHtml(d.location.source || 'unknown')}:${escapeHtml(d.location.line)}:${escapeHtml(d.location.column)}
</p>` : ''}
</article>`).join('')
            : '<p class="empty">No diagnostics reported.</p>';
    }]
]);

const jsonFallback = (container, section, reason = '') => {
    container.innerHTML = `${reason ? `
<p class="component-note">
    ${escapeHtml(reason)}
</p>` : ''}
<pre class="json-tree">
    <code>
        ${escapeHtml(JSON.stringify(section.data, null, 2))}
    </code>
</pre>`;
};

const loadRendererRegistry = async () => {
    try {
        const response = await fetch(componentsEndpoint, {
            headers: { Accept: 'application/json' },
            cache: 'no-store'
        });
        if (!response.ok) throw new Error(`Renderer registry request failed with HTTP ${response.status}`);
        const payload = await response.json();
        return new Map((Array.isArray(payload.components) ? payload.components : [])
            .filter(entry => entry && typeof entry.name === 'string' && typeof entry.module === 'string')
            .map(entry => [entry.name, entry.module]));
    } catch (cause) {
        console.warn('Unable to load Developer Tools component registry.', cause);
        return new Map();
    }
};

const renderSection = async (host, section, rendererRegistry) => {
    const container = document.createElement('div');
    host.appendChild(container);

    const componentName = typeof section.component === 'string' ? section.component : '';
    const builtIn = builtInRenderers.get(componentName);
    if (builtIn) {
        builtIn(container, section);
        return;
    }

    const moduleUrl = rendererRegistry.get(componentName);
    if (!moduleUrl) {
        jsonFallback(container, section, componentName ? `Renderer '${componentName}' is not registered.` : 'No renderer requested.');
        return;
    }

    try {
        const module = await import(moduleUrl);
        const renderer = typeof module.render === 'function'
            ? module.render
            : typeof module.default === 'function'
                ? module.default
                : null;
        if (!renderer) throw new Error(`Module '${moduleUrl}' does not export a renderer function.`);
        await renderer(container, {
            id: section.id,
            title: section.title,
            component: componentName,
            data: section.data
        });
    } catch (cause) {
        jsonFallback(container, section, cause instanceof Error ? cause.message : String(cause));
    }
};

const renderSummary = (data) => {
    const sections = Array.isArray(data.sections) ? data.sections : [];
    const items = [
        ['Schema', data.schemaVersion],
        ['Routes', Array.isArray(data.routes) ? data.routes.length : 0],
        ['Components', Array.isArray(data.components) ? data.components.length : 0],
        ['Services', Array.isArray(data.services) ? data.services.length : 0],
        ['Diagnostics', Array.isArray(data.diagnostics) ? data.diagnostics.length : 0],
        ['Sections', sections.length]
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

const renderInspection = async (data, rendererRegistry) => {
    renderSummary(data);
    const sections = Array.isArray(data.sections) ? data.sections : [];
    const host = byId('extension-sections');
    host.replaceChildren();

    for (const section of sections) {
        const panel = document.createElement('section');
        panel.className = 'panel';
        const heading = document.createElement('h2');
        heading.textContent = section.title || section.id || 'Section';
        panel.appendChild(heading);
        host.appendChild(panel);
        await renderSection(panel, section, rendererRegistry);
    }
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
        const [response, rendererRegistry] = await Promise.all([
            fetch(inspectionEndpoint, {
                headers: { Accept: 'application/json' },
                cache: 'no-store',
                signal: controller.signal
            }),
            loadRendererRegistry()
        ]);
        window.clearTimeout(timeout);
        if (!response.ok) throw new Error(`Inspection request failed with HTTP ${response.status}`);
        const data = await response.json();
        await renderInspection(data, rendererRegistry);
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
    if (!refreshButton || !byId('status-text') || !byId('error-state')) return;
    refreshButton.addEventListener('click', loadInspection);
    loadInspection();
};

if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', startDiagnostics, { once: true });
} else {
    startDiagnostics();
}
)DROGULAR_JS";

} // namespace

std::string_view pageHtml() { return PageHtml; }
std::string_view stylesheet() { return Stylesheet; }
std::string_view script() { return Script; }

} // namespace drogular::diagnostics_resources
