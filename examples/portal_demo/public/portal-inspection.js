const escapeHtml = (value) => String(value ?? '')
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#039;');

const flatten = (value, prefix = '') => {
    const rows = [];
    for (const [key, item] of Object.entries(value ?? {})) {
        const name = prefix ? `${prefix}.${key}` : key;
        if (item && typeof item === 'object' && !Array.isArray(item)) {
            rows.push(...flatten(item, name));
        } else {
            rows.push([name, Array.isArray(item) ? item.join(', ') : item]);
        }
    }
    return rows;
};

export function render(container, context) {
    const rows = flatten(context.data);
    container.innerHTML = rows.length
        ? `
<div class="table-wrap">
    <table>
        <thead>
            <tr>
                <th>Property</th>
                <th>Value</th>
            </tr>
        </thead>
        <tbody>${rows.map(([name, value]) => `
            <tr>
                <td>
                    <code>
                        ${escapeHtml(name)}
                    </code>
                </td>
                <td>
                    ${escapeHtml(value)}
                </td>
            </tr>`).join('')}
        </tbody>
    </table>
</div>`
        : '<p class="empty">No Portal data.</p>';
}
