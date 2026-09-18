#include <drogular/interactions_resources.hpp>

namespace drogular::interactions_resources {

namespace {

constexpr std::string_view Script = R"DROGULAR_JS((() => {
    const requestStates = new WeakMap();
    const stateClasses = ['dg-loading', 'dg-ready', 'dg-empty', 'dg-error'];

    const createRequestState = () => ({
        running: false,
        pending: false,
        failures: 0,
        paused: false,
        pollingIntervals: [],
        pollingDelays: [],
    });

    const requestState = (element) => {
        let state = requestStates.get(element);
        if (!state) {
            state = createRequestState();
            requestStates.set(element, state);
        }
        return state;
    };

    const connectionElements = (element) => {
        const statusSelector = element.getAttribute('dg-connection');
        const retrySelector = element.getAttribute('dg-retry');
        return {
            status: statusSelector ? document.querySelector(statusSelector) : null,
            retry: retrySelector ? document.querySelector(retrySelector) : null,
        };
    };

    const setConnectionState = (element, state, detail = {}) => {
        const { status, retry } = connectionElements(element);
        if (!status) return;

        status.setAttribute('data-dg-connection-state', state);

        const label = status.querySelector('[data-dg-connection-label]');
        const detailElement = status.querySelector('[data-dg-connection-detail]');
        if (label) {
            if (detail.label) {
                label.textContent = detail.label;
            } else if (state === 'reconnecting') {
                const base = status.dataset.dgLabelReconnecting || 'Reconnecting';
                label.textContent = `${base} ${detail.attempt || 1}/${detail.limit || 1}`;
            } else {
                const key = `dgLabel${state.charAt(0).toUpperCase()}${state.slice(1)}`;
                label.textContent = status.dataset[key] || state;
            }
        }

        if (detailElement) {
            const message = detail.detail || '';
            detailElement.hidden = !message;
            detailElement.textContent = message;
        }

        if (retry) retry.hidden = state !== 'offline';
    };

    const applyConnectionResponse = (element, target) => {
        if (!element.hasAttribute('dg-connection')) return;
        const source = target.querySelector('[data-dg-connection-state]');
        const state = source?.dataset.dgConnectionState || 'live';
        setConnectionState(element, state, {
            label: source?.dataset.dgConnectionLabel || '',
            detail: source?.dataset.dgConnectionDetail || '',
        });
    };

    const parseDelay = (token) => {
        const match = token.match(/delay:(\d+)ms/);
        return match ? Number(match[1]) : 0;
    };

    const requestParameters = (element, submitter = null) => {
        const parameters = new URLSearchParams();
        const controls = element.querySelectorAll(
            'input[name], select[name], textarea[name]'
        );

        for (const control of controls) {
            if (control.disabled || !control.name) continue;
            if (
                (control.type === 'checkbox' || control.type === 'radio') &&
                !control.checked
            ) {
                continue;
            }
            parameters.set(control.name, control.value);
        }

        if (submitter && submitter.name && !submitter.disabled) {
            parameters.set(submitter.name, submitter.value);
        }

        return parameters;
    };

    const requestUrl = (element, submitter = null) => {
        const source =
            element.getAttribute('dg-get') ||
            element.getAttribute('dg-post');
        if (!source) return null;

        const url = new URL(source, window.location.origin);
        if (element.hasAttribute('dg-get')) {
            const parameters = requestParameters(element, submitter);
            for (const [name, value] of parameters) {
                url.searchParams.set(name, value);
            }
        }
        return url;
    };

    const requestOptions = (element, submitter = null) => {
        if (!element.hasAttribute('dg-post')) {
            return {
                headers: { 'Accept': 'text/html' },
                cache: 'no-store',
            };
        }

        return {
            method: 'POST',
            headers: {
                'Accept': 'text/html',
                'Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8',
                'X-Drogular-Interaction': 'true',
            },
            body: requestParameters(element, submitter).toString(),
            cache: 'no-store',
        };
    };

    const historyUrl = (element, request) => {
        const mode = element.getAttribute('dg-history');
        if (mode !== 'replace' && mode !== 'push') return null;

        const source =
            element.getAttribute('dg-history-url') ||
            (element.tagName === 'FORM' ? element.getAttribute('action') : null) ||
            window.location.pathname;

        const url = new URL(source, window.location.origin);
        url.search = '';

        const controls = element.querySelectorAll(
            'input[name], select[name], textarea[name]'
        );

        for (const control of controls) {
            if (control.disabled || !control.name) continue;
            if (
                (control.type === 'checkbox' || control.type === 'radio') &&
                !control.checked
            ) {
                continue;
            }

            const resetValue = control.getAttribute('dg-reset-value');
            if (resetValue !== null && control.value === resetValue) continue;
            if (control.value === '') continue;

            url.searchParams.set(control.name, control.value);
        }

        for (const [name, value] of request.searchParams) {
            if (url.searchParams.has(name) || value === '') continue;

            const submitter = element.querySelector(
                `[name="${CSS.escape(name)}"]`
            );
            if (
                submitter &&
                submitter.matches('button, input[type="submit"]')
            ) {
                url.searchParams.set(name, value);
            }
        }

        return url;
    };

    const syncHistory = (element, request) => {
        const url = historyUrl(element, request);
        if (!url) return;

        const mode = element.getAttribute('dg-history');
        const next = `${url.pathname}${url.search}${url.hash}`;

        if (mode === 'push') {
            window.history.pushState(null, '', next);
        } else {
            window.history.replaceState(null, '', next);
        }
    };

    const resetForm = (element) => {
        if (element.tagName !== 'FORM') return;

        element.reset();
        element.querySelectorAll('[dg-reset-value]').forEach((control) => {
            control.value = control.getAttribute('dg-reset-value') || '';
        });
        refresh(element);
    };

    const targetFor = (element) => {
        const selector = element.getAttribute('dg-target');
        if (!selector || selector === 'this') return element;
        return element.querySelector(selector) || document.querySelector(selector);
    };

    const setState = (element, state) => {
        element.classList.remove(...stateClasses);
        element.classList.add(`dg-${state}`);
        element.setAttribute('data-dg-state', state);
        element.setAttribute('aria-busy', state === 'loading' ? 'true' : 'false');
    };

    const responseState = (html) => {
        const template = document.createElement('template');
        template.innerHTML = html;
        return template.content.querySelector('[data-dg-empty]') ? 'empty' : 'ready';
    };

    const preservedOpenState = (target) => {
        const items = Array.from(target.querySelectorAll('details[data-dg-preserve-key]'));
        return {
            initialized: items.length > 0,
            keys: new Set(items
                .filter((details) => details.open)
                .map((details) => details.getAttribute('data-dg-preserve-key'))
                .filter(Boolean)),
        };
    };

    const restoreOpenState = (target, preserved) => {
        const details = target.querySelectorAll('details[data-dg-preserve-key]');
        details.forEach((item, index) => {
            const key = item.getAttribute('data-dg-preserve-key');
            item.open = preserved.initialized ? preserved.keys.has(key) : index === 0;
        });
    };

    const pollGroupElements = (element) => {
        const group = element.getAttribute('dg-poll-group');
        if (!group) return [element];
        return Array.from(document.querySelectorAll('[dg-poll-group]'))
            .filter((candidate) => candidate.getAttribute('dg-poll-group') === group);
    };

    const stopPolling = (element) => {
        const state = requestState(element);
        for (const interval of state.pollingIntervals) window.clearInterval(interval);
        state.pollingIntervals = [];
    };

    let refresh;

    const startPolling = (element) => {
        const state = requestState(element);
        stopPolling(element);
        if (state.paused) return;
        state.pollingIntervals = state.pollingDelays.map((delay) =>
            window.setInterval(() => refresh(element), delay));
    };

    const pauseElement = (element) => {
        const state = requestState(element);
        state.paused = true;
        state.pending = false;
        stopPolling(element);
        element.setAttribute('data-dg-paused', 'true');
    };

    const pause = (element) => {
        pollGroupElements(element).forEach(pauseElement);
    };

    const resumeElement = (element) => {
        const state = requestState(element);
        state.failures = 0;
        state.pending = false;
        state.paused = false;
        element.removeAttribute('data-dg-paused');
        startPolling(element);
    };

    const resume = (element) => {
        const members = pollGroupElements(element);
        members.forEach(resumeElement);
        setConnectionState(element, 'connecting');
        members.forEach((member) => refresh(member));
    };

    refresh = async (element, submitter = null) => {
        const state = requestState(element);
        if (state.paused) return;
        if (state.running) {
            state.pending = true;
            return;
        }

        const url = requestUrl(element, submitter);
        const target = targetFor(element);
        if (!url || !target) return;

        state.running = true;
        setState(element, 'loading');
        try {
            const response = await fetch(
                url,
                requestOptions(element, submitter)
            );
            const html = await response.text();
            const postInteraction = element.hasAttribute('dg-post');

            if (!response.ok && !postInteraction) {
                throw new Error(`HTTP ${response.status}`);
            }

            if (state.paused) return;
            if (!state.pending) {
                const openState = preservedOpenState(target);
                target.innerHTML = html;
                target.querySelectorAll('[dg-get], [dg-post]').forEach(install);
                restoreOpenState(target, openState);
                if (element.hasAttribute('dg-hide-on-unavailable')) {
                    element.hidden = target.querySelector('[data-dg-unavailable]') !== null;
                }

                if (!response.ok) {
                    setState(element, 'error');
                    return;
                }

                setState(element, responseState(html));
                state.failures = 0;
                applyConnectionResponse(element, target);
                syncHistory(element, url);

                const successRefresh =
                    element.getAttribute('dg-on-success-refresh');
                if (successRefresh) {
                    document.querySelectorAll(successRefresh).forEach((root) => {
                        if (root !== element) refresh(root);
                    });
                }
            }
        } catch (_) {
            setState(element, 'error');
            const limit = Number(element.getAttribute('dg-failure-limit') || 0);
            if (limit > 0 && element.hasAttribute('dg-connection')) {
                state.failures += 1;
                if (state.failures >= limit) {
                    if (element.hasAttribute('dg-pause-on-failure')) pause(element);
                    setConnectionState(element, 'offline');
                } else {
                    setConnectionState(element, 'reconnecting', {
                        attempt: state.failures,
                        limit,
                    });
                }
            }
        } finally {
            state.running = false;
            if (state.pending && !state.paused) {
                state.pending = false;
                refresh(element);
            }
        }
    };

    const install = (element) => {
        if (element.hasAttribute('data-dg-installed')) return;
        element.setAttribute('data-dg-installed', 'true');

        const state = requestState(element);
        const trigger = element.getAttribute('dg-trigger') ||
            (element.hasAttribute('dg-post') ? 'submit' : 'load');
        const tokens = trigger.split(',').map((value) => value.trim()).filter(Boolean);

        for (const token of tokens) {
            if (token === 'load') {
                refresh(element);
                continue;
            }

            const every = token.match(/^every\s+(\d+(?:\.\d+)?)s$/);
            if (every) {
                state.pollingDelays.push(Number(every[1]) * 1000);
                continue;
            }

            const eventName = token.split(/\s+/)[0];
            if (eventName !== 'input' && eventName !== 'change') continue;

            const delay = parseDelay(token);
            let timer = null;
            element.addEventListener(eventName, () => {
                if (timer !== null) window.clearTimeout(timer);
                if (delay === 0) {
                    refresh(element);
                    return;
                }
                timer = window.setTimeout(() => refresh(element), delay);
            });
        }

        startPolling(element);

        if (element.tagName === 'FORM') {
            element.addEventListener('submit', (event) => {
                event.preventDefault();
                refresh(element, event.submitter || null);
            });

            element.querySelectorAll('[dg-reset]').forEach((control) => {
                control.addEventListener('click', (event) => {
                    event.preventDefault();
                    resetForm(element);
                });
            });
        }

        element.addEventListener('dg:resume', () => resume(element));
    };

    document.addEventListener('submit', (event) => {
        const form = event.target;
        if (!(form instanceof HTMLFormElement)) return;

        const currentUrl =
            `${window.location.pathname}${window.location.search}${window.location.hash}`;
        form.querySelectorAll('[dg-current-url]').forEach((control) => {
            control.value = currentUrl;
        });
    }, true);

    document.querySelectorAll('[dg-get], [dg-post]').forEach(install);
    document.querySelectorAll('[dg-resume]').forEach((control) => {
        control.addEventListener('click', () => {
            const selector = control.getAttribute('dg-resume');
            const target = selector ? document.querySelector(selector) : null;
            if (target) target.dispatchEvent(new CustomEvent('dg:resume'));
        });
    });
})();
)DROGULAR_JS";

} // namespace

std::string_view script() {
    return Script;
}

} // namespace drogular::interactions_resources