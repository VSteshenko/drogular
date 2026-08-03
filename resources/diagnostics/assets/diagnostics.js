const inspectionEndpoint = document.body.dataset.inspectionEndpoint || '/__drogular/inspection';
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
