#include <drogular/ui_resources.hpp>

namespace drogular::ui_resources {

namespace {

constexpr std::string_view Stylesheet = R"DROGULAR_CSS(
:root,
[data-dg-theme="light"],
[data-dg-theme="system"] {
    --dg-bg: #f5f7fb;
    --dg-surface: #ffffff;
    --dg-surface-muted: #eef2f7;
    --dg-surface-raised: #ffffff;
    --dg-text: #172033;
    --dg-text-muted: #667085;
    --dg-border: #d7dee9;
    --dg-border-strong: #b8c3d3;
    --dg-accent: #315efb;
    --dg-accent-hover: #2349d8;
    --dg-focus: #315efb;
    --dg-success: #18794e;
    --dg-success-soft: #e8f7ef;
    --dg-info: #175cd3;
    --dg-info-soft: #eaf2ff;
    --dg-warning: #a15c07;
    --dg-warning-soft: #fff4df;
    --dg-danger: #b42318;
    --dg-danger-soft: #fff0ee;
    --dg-shadow-sm: 0 1px 2px rgba(16, 24, 40, .06), 0 2px 8px rgba(16, 24, 40, .05);
    --dg-shadow-md: 0 16px 40px rgba(16, 24, 40, .10);
    --dg-radius-sm: .5rem;
    --dg-radius-md: .75rem;
    --dg-radius-lg: 1rem;
    color-scheme: light;
}

[data-dg-theme="dark"] {
    --dg-bg: #0b1020;
    --dg-surface: #121a2f;
    --dg-surface-muted: #0f1729;
    --dg-surface-raised: #17223b;
    --dg-text: #e8eefc;
    --dg-text-muted: #9eacc6;
    --dg-border: #263451;
    --dg-border-strong: #3a4b6d;
    --dg-accent: #8fb3ff;
    --dg-accent-hover: #b0c8ff;
    --dg-focus: #8fb3ff;
    --dg-success: #6ee7a8;
    --dg-success-soft: #123326;
    --dg-info: #9fc5ff;
    --dg-info-soft: #172b49;
    --dg-warning: #f0b95f;
    --dg-warning-soft: #382b16;
    --dg-danger: #f28b82;
    --dg-danger-soft: #3b2024;
    --dg-shadow-sm: 0 1px 2px rgba(0, 0, 0, .22), 0 2px 10px rgba(0, 0, 0, .18);
    --dg-shadow-md: 0 18px 50px rgba(0, 0, 0, .24);
    color-scheme: dark;
}

@media (prefers-color-scheme: dark) {
    :root:not([data-dg-theme]),
    [data-dg-theme="system"] {
        --dg-bg: #0b1020;
        --dg-surface: #121a2f;
        --dg-surface-muted: #0f1729;
        --dg-surface-raised: #17223b;
        --dg-text: #e8eefc;
        --dg-text-muted: #9eacc6;
        --dg-border: #263451;
        --dg-border-strong: #3a4b6d;
        --dg-accent: #8fb3ff;
        --dg-accent-hover: #b0c8ff;
        --dg-focus: #8fb3ff;
        --dg-success: #6ee7a8;
        --dg-success-soft: #123326;
        --dg-info: #9fc5ff;
        --dg-info-soft: #172b49;
        --dg-warning: #f0b95f;
        --dg-warning-soft: #382b16;
        --dg-danger: #f28b82;
        --dg-danger-soft: #3b2024;
        --dg-shadow-sm: 0 1px 2px rgba(0, 0, 0, .22), 0 2px 10px rgba(0, 0, 0, .18);
        --dg-shadow-md: 0 18px 50px rgba(0, 0, 0, .24);
        color-scheme: dark;
    }
}

html {
    background: var(--dg-bg);
    color: var(--dg-text);
}

body {
    margin: 0;
    background: var(--dg-bg);
    color: var(--dg-text);
    font-family: Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
}

a {
    color: var(--dg-accent);
}

a:hover {
    color: var(--dg-accent-hover);
}

button,
input,
select,
textarea {
    color: inherit;
    font: inherit;
}

input,
select,
textarea {
    border: 1px solid var(--dg-border);
    border-radius: var(--dg-radius-sm);
    background: var(--dg-surface);
    color: var(--dg-text);
}

.dg-card {
    border: 1px solid var(--dg-border);
    border-radius: var(--dg-radius-lg);
    background: var(--dg-surface);
    color: var(--dg-text);
    box-shadow: var(--dg-shadow-md);
}

.dg-card-header,
.dg-card-body,
.dg-card-footer {
    padding: 1rem 1.1rem;
}

.dg-card-header {
    border-bottom: 1px solid var(--dg-border);
}

.dg-card-header > :first-child,
.dg-card-body > :first-child,
.dg-card-footer > :first-child {
    margin-top: 0;
}

.dg-card-header > :last-child,
.dg-card-body > :last-child,
.dg-card-footer > :last-child {
    margin-bottom: 0;
}

.dg-card-footer {
    border-top: 1px solid var(--dg-border);
    background: var(--dg-surface-muted);
}

.dg-card-title {
    margin: 0;
    color: var(--dg-text);
    font-size: 1rem;
    line-height: 1.35;
}

.dg-card-subtitle {
    margin: .25rem 0 0;
    color: var(--dg-text-muted);
    font-size: .88rem;
    line-height: 1.45;
}

.dg-card-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(min(100%, 16rem), 1fr));
    gap: 1rem;
}

.dg-stack {
    display: grid;
    gap: 1.25rem;
}

.dg-collapsible {
    overflow: clip;
}

.dg-card-summary {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 1rem;
    padding: 1rem 1.1rem;
    cursor: pointer;
    list-style: none;
    user-select: none;
}

.dg-card-summary::-webkit-details-marker {
    display: none;
}

.dg-card-summary::after {
    content: "⌄";
    flex: 0 0 auto;
    color: var(--dg-text-muted);
    font-size: 1rem;
    line-height: 1;
    transition: transform .15s ease;
}

.dg-collapsible[open] > .dg-card-summary {
    border-bottom: 1px solid var(--dg-border);
}

.dg-collapsible[open] > .dg-card-summary::after {
    transform: rotate(180deg);
}

.dg-card-summary:hover {
    background: var(--dg-surface-muted);
}

.dg-card-summary:focus-visible {
    outline: 2px solid var(--dg-focus);
    outline-offset: -2px;
}

.dg-link-list {
    margin: 0;
    padding: 0;
    list-style: none;
    display: grid;
    gap: .45rem;
}

.dg-link-list a {
    display: block;
    padding: .65rem .75rem;
    border-radius: var(--dg-radius-sm);
    color: var(--dg-text);
    text-decoration: none;
}

.dg-link-list a:hover {
    background: var(--dg-surface-muted);
    color: var(--dg-accent-hover);
}

.dg-form {
    display: grid;
    gap: 1rem;
}

.dg-form-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(min(100%, 13rem), 1fr));
    gap: .9rem 1rem;
}

.dg-field {
    min-width: 0;
    display: grid;
    gap: .4rem;
}

.dg-label,
.dg-field > label {
    color: var(--dg-text);
    font-size: .86rem;
    font-weight: 650;
}

.dg-input,
.dg-select,
.dg-field input,
.dg-field select,
.dg-field textarea {
    width: 100%;
    min-width: 0;
    box-sizing: border-box;
    padding: .62rem .72rem;
    border: 1px solid var(--dg-border);
    border-radius: var(--dg-radius-sm);
    background: var(--dg-surface);
    color: var(--dg-text);
    transition: border-color .15s ease, box-shadow .15s ease, background-color .15s ease;
}

.dg-input:hover,
.dg-select:hover,
.dg-field input:hover,
.dg-field select:hover,
.dg-field textarea:hover {
    border-color: var(--dg-border-strong);
}

.dg-input:focus-visible,
.dg-select:focus-visible,
.dg-field input:focus-visible,
.dg-field select:focus-visible,
.dg-field textarea:focus-visible {
    outline: 0;
    border-color: var(--dg-focus);
    box-shadow: 0 0 0 3px color-mix(in srgb, var(--dg-focus) 18%, transparent);
}

.dg-input:disabled,
.dg-select:disabled,
.dg-field input:disabled,
.dg-field select:disabled,
.dg-field textarea:disabled {
    cursor: not-allowed;
    opacity: .65;
    background: var(--dg-surface-muted);
}

.dg-form-actions {
    display: flex;
    align-items: center;
    gap: .65rem;
    flex-wrap: wrap;
    margin-top: .35rem;
}

.dg-fieldset > .dg-form-actions {
    margin-top: 1.1rem;
}

.dg-fieldset {
    min-width: 0;
    margin: 0;
    padding: 0;
    border: 0;
}

.dg-fieldset-legend {
    margin: 0 0 1rem;
    padding: 0;
    color: var(--dg-text);
    font-size: 1rem;
    font-weight: 700;
}

.dg-table-container {
    width: 100%;
    overflow-x: auto;
    border: 1px solid var(--dg-border);
    border-radius: var(--dg-radius-md);
    background: var(--dg-surface);
}

.dg-table {
    width: 100%;
    border-collapse: collapse;
    color: var(--dg-text);
    font-size: .92rem;
}

.dg-table th,
.dg-table td {
    padding: .78rem .9rem;
    border-bottom: 1px solid var(--dg-border);
    text-align: left;
    vertical-align: middle;
}

.dg-table th {
    background: var(--dg-surface-muted);
    color: var(--dg-text-muted);
    font-size: .76rem;
    font-weight: 750;
    letter-spacing: .045em;
    text-transform: uppercase;
}

.dg-table tbody tr:last-child td {
    border-bottom: 0;
}

.dg-table tbody tr:hover td {
    background: var(--dg-surface-muted);
}

.dg-table a {
    font-weight: 650;
    text-decoration: none;
}

.dg-empty-state {
    padding: 2rem 1rem;
    color: var(--dg-text-muted);
    text-align: center;
}

.dg-table .dg-empty-state {
    background: var(--dg-surface);
}

.dg-pagination {
    display: flex;
    align-items: center;
    gap: .35rem;
    flex-wrap: wrap;
}

.dg-pagination a,
.dg-pagination strong {
    min-width: 2rem;
    box-sizing: border-box;
    padding: .38rem .55rem;
    border: 1px solid var(--dg-border);
    border-radius: var(--dg-radius-sm);
    background: var(--dg-surface);
    color: var(--dg-text);
    text-align: center;
    text-decoration: none;
}

.dg-pagination a:hover {
    border-color: var(--dg-accent);
    background: var(--dg-surface-muted);
}

.dg-pagination strong[aria-current="page"] {
    border-color: var(--dg-accent);
    background: var(--dg-info-soft);
    color: var(--dg-info);
}

.dg-button {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    padding: .45rem .7rem;
    border: 1px solid var(--dg-border-strong);
    border-radius: .65rem;
    background: var(--dg-surface-raised);
    color: var(--dg-text);
    font: inherit;
    text-decoration: none;
    cursor: pointer;
}

.dg-button:hover {
    border-color: var(--dg-accent);
    background: var(--dg-surface-muted);
}

.dg-button:focus-visible,
.dg-segmented-item:focus-visible {
    outline: 2px solid var(--dg-focus);
    outline-offset: 2px;
}

.dg-button:disabled,
.dg-segmented-item:disabled {
    cursor: default;
    opacity: .65;
}

.dg-toolbar {
    display: flex;
    align-items: center;
    gap: .65rem;
}

.dg-status {
    display: inline-flex;
    align-items: center;
    gap: .45rem;
    padding: .45rem .7rem;
    border: 1px solid var(--dg-border);
    border-radius: 999px;
    background: var(--dg-surface-muted);
    color: var(--dg-text-muted);
}

.dg-status-neutral,
.dg-status[data-dg-connection-state="idle"] {
    color: var(--dg-text-muted);
    --dg-status-dot: var(--dg-text-muted);
}
.dg-status-info,
.dg-status[data-dg-connection-state="connecting"] {
    color: var(--dg-info);
    background: var(--dg-info-soft);
    --dg-status-dot: var(--dg-info);
}
.dg-status-success,
.dg-status[data-dg-connection-state="live"] {
    color: var(--dg-success);
    background: var(--dg-success-soft);
    --dg-status-dot: var(--dg-success);
}
.dg-status-warning,
.dg-status[data-dg-connection-state="stale"],
.dg-status[data-dg-connection-state="reconnecting"] {
    color: var(--dg-warning);
    background: var(--dg-warning-soft);
    --dg-status-dot: var(--dg-warning);
}
.dg-status-danger,
.dg-status[data-dg-connection-state="offline"] {
    color: var(--dg-danger);
    background: var(--dg-danger-soft);
    --dg-status-dot: var(--dg-danger);
}

.dg-badge {
    display: inline-flex;
    align-items: center;
    border: 1px solid var(--dg-border);
    border-radius: 999px;
    padding: .22rem .55rem;
    background: var(--dg-surface-muted);
    font-size: .8rem;
    font-weight: 600;
}
.dg-badge-neutral { color: var(--dg-text-muted); }
.dg-badge-info {
    color: var(--dg-info);
    border-color: var(--dg-info);
}
.dg-badge-success {
    color: var(--dg-success);
    border-color: var(--dg-success);
}
.dg-badge-warning {
    color: var(--dg-warning);
    border-color: var(--dg-warning);
}
.dg-badge-danger {
     color: var(--dg-danger);
     border-color: var(--dg-danger);
}

.dg-segmented {
    display: inline-flex;
    align-items: center;
    gap: .2rem;
    padding: .2rem;
    border: 1px solid var(--dg-border);
    border-radius: .7rem;
    background: var(--dg-surface-muted);
}

.dg-segmented-item {
    border: 0;
    border-radius: .5rem;
    background: transparent;
    color: var(--dg-text-muted);
    padding: .35rem .6rem;
    font: inherit;
    cursor: pointer;
}
.dg-segmented-item:hover { color: var(--dg-text); }
.dg-segmented-item.is-active {
    background: var(--dg-surface-raised);
    color: var(--dg-text);
    box-shadow: var(--dg-shadow-sm);
}
.dg-segmented-item:focus-visible {
    outline-offset: 1px;
}


.dg-shell {
    min-height: 100vh;
    display: grid;
    grid-template-columns: 16rem minmax(0, 1fr);
    grid-template-rows: auto 1fr;
    background: var(--dg-bg);
}

.dg-sidebar {
    grid-column: 1;
    grid-row: 1 / span 2;
    min-width: 0;
    padding: 1.25rem 1rem;
    border-right: 1px solid var(--dg-border);
    background: var(--dg-surface);
    display: flex;
    flex-direction: column;
    gap: 1.25rem;
}

.dg-brand {
    display: flex;
    align-items: center;
    gap: .75rem;
    color: var(--dg-text);
    font-size: 1.05rem;
    font-weight: 750;
    letter-spacing: -.02em;
    text-decoration: none;
}

.dg-brand:hover {
    color: var(--dg-text);
}

.dg-brand-mark {
    width: 2rem;
    height: 2rem;
    border-radius: .65rem;
    display: inline-grid;
    place-items: center;
    flex: 0 0 auto;
    background: var(--dg-accent);
    color: var(--dg-surface);
    box-shadow: var(--dg-shadow-sm);
    font-size: .85rem;
    font-weight: 800;
}

[data-dg-theme="dark"] .dg-brand-mark {
    color: var(--dg-bg);
}

@media (prefers-color-scheme: dark) {
    [data-dg-theme="system"] .dg-brand-mark {
        color: var(--dg-bg);
    }
}

.dg-nav {
    display: flex;
    flex-direction: column;
    gap: .3rem;
}

.dg-nav-item {
    display: flex;
    align-items: center;
    min-height: 2.35rem;
    padding: .25rem .7rem;
    border-radius: var(--dg-radius-sm);
    color: var(--dg-text-muted);
    font-size: .94rem;
    font-weight: 550;
    text-decoration: none;
    transition: background-color .15s ease, color .15s ease;
}

.dg-nav-item:hover {
    background: var(--dg-surface-muted);
    color: var(--dg-text);
}

.dg-nav-item.is-active,
.dg-nav-item[aria-current="page"] {
    background: var(--dg-info-soft);
    color: var(--dg-info);
}

.dg-nav-group {
    display: grid;
    gap: .2rem;
}

.dg-nav-submenu {
    margin-left: .7rem;
    padding-left: .65rem;
    border-left: 1px solid var(--dg-border);
    display: grid;
    gap: .15rem;
}

.dg-nav-subitem {
    display: flex;
    align-items: center;
    min-height: 1.95rem;
    padding: .2rem .55rem;
    border-radius: var(--dg-radius-sm);
    color: var(--dg-text-muted);
    font-size: .84rem;
    font-weight: 500;
    text-decoration: none;
    transition: background-color .15s ease, color .15s ease;
}

.dg-nav-subitem:hover {
    background: var(--dg-surface-muted);
    color: var(--dg-text);
}

.dg-nav-subitem.is-active,
.dg-nav-subitem[aria-current="page"] {
    background: var(--dg-info-soft);
    color: var(--dg-info);
}

.dg-sidebar-footer {
    margin-top: auto;
    padding-top: 1rem;
    border-top: 1px solid var(--dg-border);
    display: grid;
    gap: .65rem;
}

.dg-sidebar-user {
    min-width: 0;
    color: var(--dg-text-muted);
    font-size: .82rem;
    line-height: 1.4;
}

.dg-sidebar-user strong {
    display: block;
    overflow: hidden;
    color: var(--dg-text);
    font-size: .92rem;
    text-overflow: ellipsis;
    white-space: nowrap;
}

.dg-topbar {
    grid-column: 2;
    grid-row: 1;
    min-width: 0;
    min-height: 4.25rem;
    padding: .75rem clamp(1rem, 3vw, 2rem);
    border-bottom: 1px solid var(--dg-border);
    background: var(--dg-surface);
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 1rem;
}

.dg-topbar-context {
    min-width: 0;
    color: var(--dg-text-muted);
    font-size: .86rem;
}

.dg-topbar-actions {
    display: flex;
    align-items: center;
    justify-content: flex-end;
    gap: .65rem;
    flex-wrap: wrap;
}

.dg-main {
    grid-column: 2;
    grid-row: 2;
    min-width: 0;
    display: flex;
    flex-direction: column;
}

.dg-page {
    width: min(100%, 78rem);
    box-sizing: border-box;
    margin: 0 auto;
    padding: clamp(1.5rem, 4vw, 3rem) clamp(1rem, 3vw, 2rem) 3rem;
    flex: 1 1 auto;
}

.dg-page-header {
    margin-bottom: 1.75rem;
}

.dg-page-kicker {
    margin: 0 0 .35rem;
    color: var(--dg-text-muted);
    font-size: .78rem;
    font-weight: 700;
    letter-spacing: .08em;
    text-transform: uppercase;
}

.dg-page-title {
    margin: 0;
    color: var(--dg-text);
    font-size: clamp(1.7rem, 3vw, 2.35rem);
    line-height: 1.12;
    letter-spacing: -.035em;
}

.dg-page-content {
    min-width: 0;
}

.dg-page-content > :first-child {
    margin-top: 0;
}

.dg-footer {
    width: min(100%, 78rem);
    box-sizing: border-box;
    margin: 0 auto;
    padding: 1rem clamp(1rem, 3vw, 2rem) 1.5rem;
    color: var(--dg-text-muted);
    font-size: .82rem;
}

@media (max-width: 760px) {
    .dg-shell {
        display: block;
    }

    .dg-sidebar {
        padding: .8rem 1rem;
        border-right: 0;
        border-bottom: 1px solid var(--dg-border);
        gap: .75rem;
    }

    .dg-nav {
        margin-inline: -1rem;
        padding-inline: 1rem;
        flex-direction: row;
        overflow-x: auto;
        scrollbar-width: thin;
    }

    .dg-nav-item {
        flex: 0 0 auto;
    }

    .dg-nav-group {
        flex: 0 0 auto;
        display: flex;
        align-items: center;
        gap: .35rem;
    }

    .dg-nav-submenu {
        margin-left: 0;
        padding-left: 0;
        border-left: 0;
        display: flex;
        gap: .2rem;
    }

    .dg-nav-subitem {
        flex: 0 0 auto;
        white-space: nowrap;
    }

    .dg-sidebar-footer {
        display: none;
    }

    .dg-topbar {
        padding: .7rem 1rem;
        align-items: flex-start;
        flex-direction: column;
    }

    .dg-topbar-actions {
        justify-content: flex-start;
    }

    .dg-page {
        padding-top: 1.5rem;
    }
}
)DROGULAR_CSS";

} // namespace

std::string_view stylesheet() {
    return Stylesheet;
}

} // namespace drogular::ui_resources
