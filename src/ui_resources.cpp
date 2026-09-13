#include <drogular/ui_resources.hpp>

namespace drogular::ui_resources {

namespace {

constexpr std::string_view Stylesheet = R"DROGULAR_CSS(
.dg-card {
    border: 1px solid #263451;
    border-radius: 1rem;
    background: #121a2f;
    box-shadow: 0 18px 50px rgba(0, 0, 0, .18);
}

.dg-button {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    padding: .45rem .7rem;
    border: 1px solid #30405f;
    border-radius: .65rem;
    background: #151f35;
    color: #dce7ff;
    font: inherit;
    text-decoration: none;
    cursor: pointer;
}

.dg-button:hover { background: #1d2a46; }
.dg-button:focus-visible {
    outline: 2px solid #8fb3ff;
    outline-offset: 2px;
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
    border: 1px solid #30405f;
    border-radius: 999px;
    color: #b9c9e8;
}

.dg-status-neutral,
.dg-status[data-dg-connection-state="idle"] {
    color: #b8c1d8;
    --dg-status-dot: #8fa0bd;
}
.dg-status-info,
.dg-status[data-dg-connection-state="connecting"] {
    color: #9fc5ff;
    --dg-status-dot: #9fc5ff;
}
.dg-status-success,
.dg-status[data-dg-connection-state="live"] {
    color: #b9c9e8;
    --dg-status-dot: #6ee7a8;
}
.dg-status-warning,
.dg-status[data-dg-connection-state="stale"],
.dg-status[data-dg-connection-state="reconnecting"] {
    color: #f6cf84;
    --dg-status-dot: #f0b95f;
}
.dg-status-danger,
.dg-status[data-dg-connection-state="offline"] {
    color: #f0b7b7;
    --dg-status-dot: #f06f6f;
}

.dg-badge {
    display: inline-flex;
    align-items: center;
    border: 1px solid #31415f;
    border-radius: 999px;
    padding: .22rem .55rem;
    font-size: .8rem;
    font-weight: 600;
}
.dg-badge-neutral { color: #b8c1d8; border-color: #40506f; }
.dg-badge-info { color: #9fc5ff; border-color: #315071; }
.dg-badge-success { color: #bfe8c9; border-color: #4f7657; }
.dg-badge-warning { color: #f6cf84; border-color: #66522c; }
.dg-badge-danger { color: #f0b7b7; border-color: #70484e; }

.dg-segmented {
    display: inline-flex;
    align-items: center;
    gap: .2rem;
    padding: .2rem;
    border: 1px solid #263451;
    border-radius: .7rem;
    background: #0c1427;
}

.dg-segmented-item {
    border: 0;
    border-radius: .5rem;
    background: transparent;
    color: #9aa8c3;
    padding: .35rem .6rem;
    font: inherit;
    cursor: pointer;
}
.dg-segmented-item:hover { color: #dce7ff; }
.dg-segmented-item.is-active {
    background: #213153;
    color: #eef2ff;
}
.dg-segmented-item:focus-visible {
    outline: 2px solid #8fb3ff;
    outline-offset: 1px;
}
)DROGULAR_CSS";

} // namespace

std::string_view stylesheet() {
    return Stylesheet;
}

} // namespace drogular::ui_resources