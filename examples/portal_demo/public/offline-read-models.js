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

    const locale = () => document.documentElement.lang || "en";

    const normalizedKeyForLocale = (url, representationLocale) => {
        const normalized = new URL(url, window.location.origin);
        normalized.hash = "";
        normalized.searchParams.sort();
        return `${scope()}|${representationLocale}|GET|${normalized.pathname}${normalized.search}`;
    };

    const normalizedKey = (url) => normalizedKeyForLocale(url, locale());

    const shellKey = (representationLocale, path = window.location.pathname) =>
        `${scope()}|${representationLocale}|SHELL|${new URL(path, window.location.origin).pathname}`;

    const message = (name, fallback) =>
        document.querySelector("[data-dg-offline-read-i18n]")?.getAttribute(`data-${name}`) || fallback;

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
            locale: locale(),
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

    const readRepresentationForLocale = (url, representationLocale) =>
        withStore("readonly", (store) => store.get(normalizedKeyForLocale(url, representationLocale)));

    const storeCurrentShell = async () => {
        const shell = document.querySelector("[data-dg-offline-shell]");
        if (!shell) return;
        const representationLocale = locale();
        await withStore("readwrite", (store) => store.put({
            key: shellKey(representationLocale),
            kind: "shell",
            locale: representationLocale,
            path: window.location.pathname,
            storedAt: new Date().toISOString(),
            html: shell.outerHTML,
        }));
    };

    const readShell = (representationLocale) =>
        withStore("readonly", (store) => store.get(shellKey(representationLocale)));

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
        const records = (await readRepresentations())
            .filter((record) => record.key.startsWith(`${scope()}|${locale()}|GET|`))
            .filter((record) => record.locale === locale())
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
        status.textContent = message(
            "status",
            "Offline · Read only · Last updated {time}"
        ).replace("{time}", time);
        status.hidden = false;
    };

    const formState = (element) => {
        if (!(element instanceof HTMLFormElement)) return null;
        const parameters = new URLSearchParams(new FormData(element));
        return parameters;
    };

    const applyFormState = (element, parameters) => {
        if (!(element instanceof HTMLFormElement) || !parameters) return;
        element.querySelectorAll("input[name], select[name], textarea[name]").forEach((control) => {
            if (control.type === "checkbox" || control.type === "radio") {
                control.checked = parameters.getAll(control.name).includes(control.value);
                return;
            }
            if (parameters.has(control.name)) control.value = parameters.get(control.name);
        });
    };

    const requestUrlFor = (element) => {
        const source = element?.getAttribute("dg-get");
        if (!source) return null;
        const url = new URL(source, window.location.origin);
        url.search = window.location.search;
        return url.toString();
    };

    let offlineSelectedLocale = null;

    const restoreOfflineLocale = async (targetLocale) => {
        const currentReader = document.querySelector("[dg-get][dg-offline-read]");
        const state = formState(currentReader);
        const shellRecord = await readShell(targetLocale);
        if (!shellRecord) return false;

        const template = document.createElement("template");
        template.innerHTML = shellRecord.html.trim();
        const replacement = template.content.firstElementChild;
        const currentShell = document.querySelector("[data-dg-offline-shell]");
        if (!replacement || !currentShell) return false;

        currentShell.replaceWith(replacement);
        document.documentElement.lang = targetLocale;
        offlineSelectedLocale = targetLocale;

        const reader = document.querySelector("[dg-get][dg-offline-read]");
        applyFormState(reader, state);

        if (reader) {
            const url = requestUrlFor(reader);
            const record = url ? await readRepresentationForLocale(url, targetLocale) : null;
            const targetSelector = reader.getAttribute("dg-target");
            const target = targetSelector ? reader.querySelector(targetSelector) : null;
            if (record && target) {
                target.innerHTML = record.html;
                setCached(reader, record.storedAt);
            } else {
                document.documentElement.setAttribute("data-dg-data-state", "cached");
                document.documentElement.setAttribute("data-dg-mode", "read-only");
            }
            renderHistory(reader).catch(() => {});
        }
        return true;
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
                setConnectionState("live");
                if (element) setLive(element);
            }
            return response;
        } catch (error) {
            try {
                const record = await readRepresentation(url);
                if (record) {
                    setConnectionState("offline");
                    if (element) setCached(element, record.storedAt);
                    return cachedResponse(record);
                }
            } catch (_) {
                // Preserve the original network failure when storage is unavailable.
            }
            throw error;
        }
    };

    const setConnectionState = (state) =>
        document.documentElement.setAttribute("data-dg-connection-state", state);

    window.addEventListener("offline", () => {
        setConnectionState("offline");
        document.documentElement.setAttribute("data-dg-mode", "read-only");
    });

    const reconcileOfflineLocale = async () => {
        if (!offlineSelectedLocale) return false;

        const redirect = `${window.location.pathname}${window.location.search}`;
        const body = new URLSearchParams({
            language: offlineSelectedLocale,
            redirect,
        });

        try {
            await nativeFetch("/language", {
                method: "POST",
                credentials: "same-origin",
                headers: {
                    "Content-Type": "application/x-www-form-urlencoded;charset=UTF-8",
                },
                body,
                redirect: "manual",
            });
        } catch (_) {
            setConnectionState("offline");
            return false;
        }

        // The server has accepted the user's last offline locale choice. A normal
        // navigation now lets the server render the authoritative representation.
        window.location.assign(redirect);
        return true;
    };

    window.addEventListener("online", () => {
        setConnectionState("reconnecting");
        if (offlineSelectedLocale) {
            reconcileOfflineLocale().catch(() => {
                setConnectionState("offline");
            });
            return;
        }
        if (document.documentElement.getAttribute("data-dg-data-state") !== "cached") {
            setConnectionState("live");
            document.documentElement.setAttribute("data-dg-mode", "read-write");
            return;
        }
        document.querySelectorAll("[dg-get][dg-offline-read]").forEach((element) => {
            if (element instanceof HTMLFormElement) element.requestSubmit();
        });
    });

    document.addEventListener("DOMContentLoaded", () => {
        setConnectionState("live");
        storeCurrentShell().catch(() => {});
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

        if (form.hasAttribute("data-dg-offline-language") &&
            document.documentElement.getAttribute("data-dg-connection-state") === "offline"
        ) {
            event.preventDefault();
            event.stopImmediatePropagation();
            const targetLocale = new FormData(form).get("language");
            if (typeof targetLocale === "string" && targetLocale) {
                restoreOfflineLocale(targetLocale).catch(() => {});
            }
            return;
        }

        if (document.documentElement.getAttribute("data-dg-mode") === "read-only" &&
            form.method.toUpperCase() !== "GET"
        ) {
            event.preventDefault();
            event.stopImmediatePropagation();
        }
    }, true);
})();