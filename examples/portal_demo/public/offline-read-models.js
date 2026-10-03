(() => {
    // Portal Demo experiment: exact-request Offline Read Models.
    // This intentionally lives outside Drogular Interactions until the
    // behavior has been validated by more than one Portal feature.
    const locale = () => document.documentElement.lang || "en";

    const message = (name, fallback) =>
        document.querySelector("[data-dg-offline-read-i18n]")?.getAttribute(`data-${name}`) || fallback;

    // Temporary Portal migration bridge: framework-owned readers keep their UX
    // here while request/cache lifecycle ownership stays in Drogular Interactions.
    // This bridge is presentation-only and never intercepts framework requests.
    const readFrameworkRepresentations = () => new Promise((resolve, reject) => {
        const request = indexedDB.open("drogular-offline-representations", 1);
        request.onsuccess = () => {
            const database = request.result;
            const transaction = database.transaction("representations", "readonly");
            const storeRequest = transaction.objectStore("representations").getAll();
            storeRequest.onsuccess = () => resolve(storeRequest.result);
            storeRequest.onerror = () => reject(storeRequest.error);
            transaction.oncomplete = () => database.close();
            transaction.onabort = () => database.close();
        };
        request.onerror = () => reject(request.error);
    });

    const frameworkRecordsFor = async (element) => {
        const source = element.getAttribute("dg-get");
        if (!source) return [];
        const sourcePath = new URL(source, window.location.origin).pathname;
        const frameworkScope = sessionStorage.getItem("drogular.offline.scope");
        if (!frameworkScope) return [];

        return (await readFrameworkRepresentations())
            .filter((record) => record.identity?.kind === "fragment")
            .filter((record) => record.identity?.scope?.kind === "session")
            .filter((record) => record.identity?.scope?.key === frameworkScope)
            .filter((record) => (record.identity?.context?.locale || "") === locale())
            .filter((record) => new URL(record.identity.requestKey, window.location.origin).pathname === sourcePath)
            .map((record) => ({
                ...record,
                url: new URL(record.identity.requestKey, window.location.origin).toString(),
                locale: record.identity.context?.locale || "",
                storedAt: new Date(record.storedAt).toISOString(),
            }));
    };

    const filterParameters = (record) => {
        const url = new URL(record.url);
        url.searchParams.delete("page");
        return url.searchParams;
    };

    const normalizedFilterIdentity = (parameters) => {
        const normalized = new URLSearchParams(parameters);
        normalized.sort();
        return normalized.toString();
    };

    const filterIdentity = (record) => normalizedFilterIdentity(filterParameters(record));

    const currentFilterIdentity = (element) => {
        const parameters = new URLSearchParams(new FormData(element));
        parameters.delete("page");
        return normalizedFilterIdentity(parameters);
    };

    const controlFor = (element, name) =>
        Array.from(element.querySelectorAll("input[name], select[name], textarea[name]"))
            .find((control) => control.name === name) || null;

    const valueLabel = (element, name, value) => {
        const control = controlFor(element, name);
        if (control instanceof HTMLSelectElement) {
            const option = Array.from(control.options).find((candidate) => candidate.value === value);
            return option?.textContent?.trim() || value;
        }
        return value;
    };

    const fieldLabel = (element, name) => {
        const control = controlFor(element, name);
        if (!control?.id) return name;
        return element.querySelector(`label[for="${CSS.escape(control.id)}"]`)?.textContent?.trim() || name;
    };

    const resetValue = (element, name) =>
        controlFor(element, name)?.getAttribute("dg-reset-value") || "";

    const historyLabel = (element, record) => {
        const parameters = filterParameters(record);
        const parts = [];
        for (const [name, value] of parameters) {
            if (!value || value === resetValue(element, name)) continue;
            const label = valueLabel(element, name, value);
            if (name === "search") parts.push(`Search: “${label}”`);
            else if (name === "sort" || name === "direction") parts.push(`${fieldLabel(element, name)}: ${label}`);
            else parts.push(label);
        }
        return parts.length
            ? parts.join(" · ")
            : (element.getAttribute("dg-offline-read-empty-label") || message("empty-label", "All items"));
    };

    const applyHistoryRecord = (element, record) => {
        const parameters = filterParameters(record);
        element.querySelectorAll("input[name], select[name], textarea[name]").forEach((control) => {
            if (control.type === "checkbox" || control.type === "radio") {
                control.checked = parameters.getAll(control.name).includes(control.value);
                return;
            }
            control.value = parameters.has(control.name)
                ? parameters.get(control.name)
                : (control.getAttribute("dg-reset-value") || "");
        });
        element.requestSubmit();
    };

    const relativeAge = (storedAt) => {
        const ageSeconds = Math.max(0, Math.floor((Date.now() - new Date(storedAt).getTime()) / 1000));
        if (!Number.isFinite(ageSeconds)) return message("cached", "cached");
        if (ageSeconds < 60) return message("cached-just-now", "cached just now");
        const minutes = Math.floor(ageSeconds / 60);
        if (minutes < 60) return message("cached-minutes", "cached {count} min ago").replace("{count}", minutes);
        const hours = Math.floor(minutes / 60);
        if (hours < 24) return message("cached-hours", "cached {count} h ago").replace("{count}", hours);
        const days = Math.floor(hours / 24);
        return message("cached-days", "cached {count} d ago").replace("{count}", days);
    };

    const historyFor = (element) => {
        let history = element.querySelector("[data-dg-offline-read-history]");
        if (history) return history;

        history = document.createElement("details");
        history.className = "dg-card dg-collapsible";
        history.setAttribute("data-dg-offline-read-history", "");
        history.innerHTML = `
            <summary class="dg-card-summary">
                <span class="dg-card-title">${message("history-title", "Cached filters")}</span>
            </summary>
            <div class="dg-card-body">
                <div class="dg-stack" data-dg-offline-read-history-items></div>
            </div>`;

        const targetSelector = element.getAttribute("dg-target");
        const target = targetSelector ? element.querySelector(targetSelector) : null;
        element.insertBefore(history, target || null);
        return history;
    };

    const renderHistory = async (element) => {
        const history = historyFor(element);
        const items = history.querySelector("[data-dg-offline-read-history-items]");
        if (!items) return;

        const source = element.getAttribute("dg-get");
        const sourcePath = source ? new URL(source, window.location.origin).pathname : "";
        const records = (await frameworkRecordsFor(element))
            .filter((record) => new URL(record.url).pathname === sourcePath)
            .sort((left, right) => right.storedAt.localeCompare(left.storedAt));

        const groups = new Map();
        for (const record of records) {
            const identity = filterIdentity(record);
            let group = groups.get(identity);
            if (!group) {
                group = { identity, newest: record, records: [] };
                groups.set(identity, group);
            }
            group.records.push(record);
        }

        const currentIdentity = currentFilterIdentity(element);
        items.replaceChildren();
        history.hidden = groups.size === 0;
        for (const group of groups.values()) {
            const button = document.createElement("button");
            button.type = "button";
            button.className = "dg-button";
            button.setAttribute("data-dg-offline-read-history-item", "");
            if (group.identity === currentIdentity) button.setAttribute("aria-current", "true");

            const label = historyLabel(element, group.newest);
            const pageCount = new Set(group.records.map((record) => new URL(record.url).searchParams.get("page") || "1")).size;
            const pages = `${pageCount} ${pageCount === 1
                ? message("page", "page")
                : message("pages", "pages")}`;
            button.textContent = `${label} · ${pages} · ${relativeAge(group.newest.storedAt)}`;
            button.title = `${message("saved", "Saved")} ${new Date(group.newest.storedAt).toLocaleString()}`;
            button.addEventListener("click", () => applyHistoryRecord(element, group.newest));
            items.append(button);
        }
    };

    const clearRepresentations = async () => {
        const request = indexedDB.open("drogular-offline-representations", 1);
        await new Promise((resolve, reject) => {
            request.onsuccess = () => {
                const database = request.result;
                const transaction = database.transaction("representations", "readwrite");
                transaction.objectStore("representations").clear();
                transaction.oncomplete = () => {
                    database.close();
                    resolve();
                };
                transaction.onerror = () => reject(transaction.error);
                transaction.onabort = () => reject(transaction.error);
            };
            request.onerror = () => reject(request.error);
        });
        sessionStorage.removeItem("drogular.offline.scope");
    };

    const statusFor = (element) => {
        let status = element.querySelector("[data-dg-offline-read-status]");
        if (status) return status;

        status = document.createElement("div");
        status.className = "dg-status dg-status-warning";
        status.setAttribute("data-dg-offline-read-status", "");
        status.setAttribute("role", "status");
        status.hidden = true;
        element.prepend(status);
        return status;
    };

    const setCached = (element, storedAt) => {
        document.documentElement.setAttribute("data-dg-data-state", "cached");
        document.documentElement.setAttribute("data-dg-mode", "read-only");
        const status = statusFor(element);
        const date = new Date(storedAt);
        const time = Number.isNaN(date.getTime()) ? storedAt : date.toLocaleString();
        status.textContent = message(
            "status",
            "Offline · Read only · Last updated {time}"
        ).replace("{time}", time);
        status.hidden = false;
    };

    const renderFrameworkOfflineState = async (state) => {
        const readers = document.querySelectorAll(
            '[dg-get][dg-offline-read][dg-offline-runtime="framework"]'
        );
        for (const element of readers) {
            renderHistory(element).catch(() => {});
            if (state.data !== "cached" || state.capability !== "read-only") {
                element.querySelector("[data-dg-offline-read-status]")?.remove();
                continue;
            }
            const records = await frameworkRecordsFor(element);
            const newest = records.sort((left, right) =>
                right.storedAt.localeCompare(left.storedAt))[0];
            if (newest) setCached(element, newest.storedAt);
        }
    };

    document.addEventListener("dg:offline-representation-stored", () => {
        document.querySelectorAll(
            '[dg-get][dg-offline-read][dg-offline-runtime="framework"]'
        ).forEach((element) => {
            renderHistory(element).catch(() => {});
        });
    });

    document.addEventListener("dg:offline-state", (event) => {
        renderFrameworkOfflineState(event.detail || {}).catch(() => {});
    });

    document.addEventListener("DOMContentLoaded", () => {
        document.querySelectorAll("[dg-get][dg-offline-read]").forEach((element) => {
            renderHistory(element).catch(() => {});
        });
    });

    document.addEventListener("submit", (event) => {
        const form = event.target;
        if (!(form instanceof HTMLFormElement)) return;

        if (form.hasAttribute("data-dg-offline-clear")) {
            clearRepresentations().catch(() => {});
            return;
        }

        if (document.documentElement.getAttribute("data-dg-mode") === "read-only" &&
            form.method.toUpperCase() !== "GET" &&
            !form.hasAttribute("dg-offline-locale")
        ) {
            event.preventDefault();
            event.stopImmediatePropagation();
        }
    }, true);
})();