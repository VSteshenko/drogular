(() => {
    const requestStates = new WeakMap();
    const stateClasses = ['dg-loading', 'dg-ready', 'dg-empty', 'dg-error'];

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

    const refresh = async (element) => {
        const state = requestStates.get(element) || { running: false, pending: false };
        requestStates.set(element, state);
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
            if (!state.pending) {
                target.innerHTML = html;
                setState(element, responseState(html));
            }
        } catch (_) {
            if (!state.pending) setState(element, 'error');
        } finally {
            state.running = false;
            if (state.pending) {
                state.pending = false;
                refresh(element);
            }
        }
    };

    const install = (element) => {
        const trigger = element.getAttribute('dg-trigger') || 'load';
        const tokens = trigger.split(',').map((value) => value.trim()).filter(Boolean);

        for (const token of tokens) {
            if (token === 'load') {
                refresh(element);
                continue;
            }

            const every = token.match(/^every\s+(\d+(?:\.\d+)?)s$/);
            if (every) {
                window.setInterval(() => refresh(element), Number(every[1]) * 1000);
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

        if (element.tagName === 'FORM') {
            element.addEventListener('submit', (event) => {
                event.preventDefault();
                refresh(element);
            });
        }
    };

    document.querySelectorAll('[dg-get]').forEach(install);
})();
