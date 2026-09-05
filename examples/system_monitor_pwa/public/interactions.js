(() => {
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

        status.classList.toggle('status-stale', state === 'stale');
        status.classList.toggle('status-reconnecting', state === 'connecting' || state === 'reconnecting');
        status.classList.toggle('status-offline', state === 'offline');

        const label = status.querySelector('[data-monitor-status-label]');
        const detailElement = status.closest('.connection-status')?.querySelector('[data-monitor-status-detail]') || null;
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

    const requestUrl = (element) => {
        const source = element.getAttribute('dg-get');
        if (!source) return null;

        const url = new URL(source, window.location.origin);
        const controls = element.querySelectorAll('input[name], select[name], textarea[name]');
        for (const control of controls) {
            if (control.disabled || !control.name) continue;
            if ((control.type === 'checkbox' || control.type === 'radio') && !control.checked) continue;
            url.searchParams.set(control.name, control.value);
        }
        return url;
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

    refresh = async (element) => {
        const state = requestState(element);
        if (state.paused) return;
        if (state.running) {
            state.pending = true;
            return;
        }

        const url = requestUrl(element);
        const target = targetFor(element);
        if (!url || !target) return;

        state.running = true;
        setState(element, 'loading');
        try {
            const response = await fetch(url, {
                headers: { 'Accept': 'text/html' },
                cache: 'no-store',
            });
            if (!response.ok) throw new Error(`HTTP ${response.status}`);
            const html = await response.text();
            if (state.paused) return;
            if (!state.pending) {
                const openState = preservedOpenState(target);
                target.innerHTML = html;
                restoreOpenState(target, openState);
                element.hidden = target.querySelector('[data-dg-unavailable]') !== null;
                setState(element, responseState(html));
                state.failures = 0;
                applyConnectionResponse(element, target);
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
        const state = requestState(element);
        const trigger = element.getAttribute('dg-trigger') || 'load';
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
                refresh(element);
            });
        }

        element.addEventListener('dg:resume', () => resume(element));
    };

    document.querySelectorAll('[dg-get]').forEach(install);
    document.querySelectorAll('[dg-resume]').forEach((control) => {
        control.addEventListener('click', () => {
            const selector = control.getAttribute('dg-resume');
            const target = selector ? document.querySelector(selector) : null;
            if (target) target.dispatchEvent(new CustomEvent('dg:resume'));
        });
    });
})();
