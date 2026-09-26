(() => {
    // Portal Demo experiment: exact-request Offline Read Models.
    // This intentionally lives outside Drogular Interactions until the
    // behavior has been validated by more than one Portal feature.
    const DB_NAME = "drogular-portal-offline-read-models";
    const DB_VERSION = 1;
    const STORE_NAME = "representations";
    const nativeFetch = window.fetch.bind(window);

    const openDatabase = () => new Promise((resolve, reject) => {
        const request = indexedDB.open(DB_NAME, DB_VERSION);
        request.onupgradeneeded = () => {
            const database = request.result;
            if (!database.objectStoreNames.contains(STORE_NAME)) {
                database.createObjectStore(STORE_NAME, { keyPath: "key" });
            }
        };
        request.onsuccess = () => resolve(request.result);
        request.onerror = () => reject(request.error);
    });

    const withStore = async (mode, operation) => {
        const database = await openDatabase();
        return new Promise((resolve, reject) => {
            const transaction = database.transaction(STORE_NAME, mode);
            const store = transaction.objectStore(STORE_NAME);
            const request = operation(store);
            request.onsuccess = () => resolve(request.result);
            request.onerror = () => reject(request.error);
            transaction.oncomplete = () => database.close();
            transaction.onabort = () => database.close();
        });
    };

    const scope = () => {
        let value = sessionStorage.getItem("dg.portal.offlineScope");
        if (!value) {
            value = crypto.randomUUID();
            sessionStorage.setItem("dg.portal.offlineScope", value);
        }
        return value;
    };

    const normalizedKey = (url) => {
        const normalized = new URL(url, window.location.origin);
        normalized.hash = "";
        normalized.searchParams.sort();
        return `${scope()}|GET|${normalized.pathname}${normalized.search}`;
    };

    const eligibleElement = (url) => {
        const requestUrl = new URL(url, window.location.origin);
        return Array.from(document.querySelectorAll("[dg-get][dg-offline-read]"))
            .find((element) => {
                const source = element.getAttribute("dg-get");
                if (!source) return false;
                return new URL(source, window.location.origin).pathname === requestUrl.pathname;
            }) || null;
    };

    const isEligibleRequest = (input, init = {}) => {
        const request = input instanceof Request ? input : null;
        const method = (init.method || request?.method || "GET").toUpperCase();
        if (method !== "GET") return false;

        const headers = new Headers(init.headers || request?.headers || {});
        if (headers.get("X-Drogular-Interaction") !== "true") return false;

        const url = request?.url || input;
        return eligibleElement(url) !== null;
    };

    const storeRepresentation = async (url, response) => {
        const html = await response.clone().text();
        const record = {
            key: normalizedKey(url),
            url: new URL(url, window.location.origin).toString(),
            storedAt: new Date().toISOString(),
            contentType: response.headers.get("Content-Type") || "text/html;charset=UTF-8",
            html,
        };
        await withStore("readwrite", (store) => store.put(record));
        const element = eligibleElement(url);
        if (element) renderHistory(element).catch(() => {});
    };

    const readRepresentation = (url) =>
        withStore("readonly", (store) => store.get(normalizedKey(url)));

    const readRepresentations = () =>
        withStore("readonly", (store) => store.getAll());

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
            : (element.getAttribute("dg-offline-read-empty-label") || "All items");
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
        if (!Number.isFinite(ageSeconds)) return "cached";
        if (ageSeconds < 60) return "cached just now";
        const minutes = Math.floor(ageSeconds / 60);
        if (minutes < 60) return `cached ${minutes} min ago`;
        const hours = Math.floor(minutes / 60);
        if (hours < 24) return `cached ${hours} h ago`;
        const days = Math.floor(hours / 24);
        return `cached ${days} d ago`;
    };

    const historyFor = (element) => {
        let history = element.querySelector("[data-dg-offline-read-history]");
        if (history) return history;

        history = document.createElement("details");
        history.className = "dg-card dg-collapsible";
        history.setAttribute("data-dg-offline-read-history", "");
        history.innerHTML = `
            <summary class="dg-card-summary">
                <span class="dg-card-title">Cached filters</span>
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
        const records = (await readRepresentations())
            .filter((record) => record.key.startsWith(`${scope()}|GET|`))
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
            const pages = `${pageCount} ${pageCount === 1 ? "page" : "pages"}`;
            button.textContent = `${label} · ${pages} · ${relativeAge(group.newest.storedAt)}`;
            button.title = `Saved ${new Date(group.newest.storedAt).toLocaleString()}`;
            button.addEventListener("click", () => applyHistoryRecord(element, group.newest));
            items.append(button);
        }
    };

    const clearRepresentations = async () => {
        try {
            await withStore("readwrite", (store) => store.clear());
        } finally {
            sessionStorage.removeItem("dg.portal.offlineScope");
        }
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

    const setLive = (element) => {
        document.documentElement.setAttribute("data-dg-data-state", "live");
        document.documentElement.setAttribute("data-dg-mode", "read-write");
        element.querySelector("[data-dg-offline-read-status]")?.remove();
        renderHistory(element).catch(() => {});
    };

    const setCached = (element, storedAt) => {
        document.documentElement.setAttribute("data-dg-data-state", "cached");
        document.documentElement.setAttribute("data-dg-mode", "read-only");
        const status = statusFor(element);
        const date = new Date(storedAt);
        const time = Number.isNaN(date.getTime()) ? storedAt : date.toLocaleString();
        status.textContent = `Offline · Read only · Last updated ${time}`;
        status.hidden = false;
    };

    const cachedResponse = (record) => new Response(record.html, {
        status: 200,
        headers: {
            "Content-Type": record.contentType,
            "X-Drogular-Offline-Read-Model": "cached",
        },
    });

    window.fetch = async (input, init = {}) => {
        if (!isEligibleRequest(input, init)) {
            return nativeFetch(input, init);
        }

        const url = input instanceof Request ? input.url : input;
        const element = eligibleElement(url);

        try {
            const response = await nativeFetch(input, init);
            if (response.ok && !response.redirected) {
                storeRepresentation(url, response).catch(() => {});
                if (element) setLive(element);
            }
            return response;
        } catch (error) {
            try {
                const record = await readRepresentation(url);
                if (record) {
                    if (element) setCached(element, record.storedAt);
                    return cachedResponse(record);
                }
            } catch (_) {
                // Preserve the original network failure when storage is unavailable.
            }
            throw error;
        }
    };

    window.addEventListener("online", () => {
        if (document.documentElement.getAttribute("data-dg-data-state") !== "cached") return;
        document.querySelectorAll("[dg-get][dg-offline-read]").forEach((element) => {
            if (element instanceof HTMLFormElement) element.requestSubmit();
        });
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

        if (
            document.documentElement.getAttribute("data-dg-mode") === "read-only" &&
            form.method.toUpperCase() !== "GET"
        ) {
            event.preventDefault();
            event.stopImmediatePropagation();
        }
    }, true);
})();