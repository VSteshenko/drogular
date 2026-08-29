(() => {
    const POLL_INTERVAL_MS = 2000;
    const GPIO_POLL_INTERVAL_MS = 30000;
    const MAX_RECONNECT_ATTEMPTS = 3;

    let reconnectAttempts = 0;
    let pollTimer = null;
    let gpioPollTimer = null;
    let pollingStopped = false;

    const field = (name) => document.querySelector(`[data-monitor-field="${name}"]`);
    const setText = (name, value) => {
        const element = field(name);
        if (element) element.textContent = value;
    };
    const setProgress = (name, value) => {
        const element = field(name);
        if (element) element.value = Math.max(0, Math.min(100, value));
    };

    const formatPercent = (value) => `${Number(value).toFixed(1)}%`;
    const formatBytes = (bytes) => {
        const units = ["B", "KiB", "MiB", "GiB", "TiB"];
        let value = Number(bytes);
        let unit = 0;
        while (value >= 1024 && unit < units.length - 1) {
            value /= 1024;
            unit += 1;
        }
        const precision = unit === 0 ? 0 : 1;
        return `${value.toFixed(precision)} ${units[unit]}`;
    };
    const formatDuration = (seconds) => {
        let remaining = Math.max(0, Math.floor(Number(seconds)));
        const days = Math.floor(remaining / 86400);
        remaining %= 86400;
        const hours = Math.floor(remaining / 3600);
        remaining %= 3600;
        const minutes = Math.floor(remaining / 60);
        if (days > 0) {
            return `${days}d ${hours}h ${minutes}m`;
        }
        if (hours > 0) {
            return `${hours}h ${minutes}m`;
        }
        return `${minutes}m`;
    };

    const formatAge = (milliseconds) => {
        const seconds = Math.max(0, Math.floor(Number(milliseconds) / 1000));
        if (seconds < 60) {
            return `${seconds}s`;
        }

        const minutes = Math.floor(seconds / 60);
        if (minutes < 60) {
            return `${minutes}m`;
        }

        const hours = Math.floor(minutes / 60);
        return `${hours}h`;
    };

    const setConnectionState = (state, monitor = null) => {
        const status = document.querySelector('[data-monitor-status]');
        const retry = document.querySelector('[data-monitor-retry]');
        const detail = document.querySelector('[data-monitor-status-detail]');
        if (!status) {
            return;
        }

        status.classList.toggle('status-stale', state === 'stale');
        status.classList.toggle('status-reconnecting', state === 'connecting' || state === 'reconnecting');
        status.classList.toggle('status-offline', state === 'offline');

        const label = status.querySelector('[data-monitor-status-label]');
        if (label) {
            if (state === 'live') {
                label.textContent = 'Live';
            } else if (state === 'stale') {
                label.textContent = `Stale · ${formatAge(monitor?.snapshotAgeMs ?? 0)} old`;
            } else if (state === 'connecting') {
                label.textContent = 'Connecting';
            } else if (state === 'reconnecting') {
                label.textContent = `Reconnecting ${reconnectAttempts}/${MAX_RECONNECT_ATTEMPTS}`;
            } else {
                label.textContent = 'Offline';
            }
        }

        if (detail) {
            if (state === 'stale') {
                detail.hidden = false;
                detail.textContent = 'Monitoring target unavailable; showing the last successful snapshot.';
            } else {
                detail.hidden = true;
                detail.textContent = '';
            }
        }

        if (retry) {
            retry.hidden = state !== 'offline';
        }
    };

    const schedulePoll = () => {
        if (pollingStopped) {
            return;
        }
        pollTimer = window.setTimeout(poll, POLL_INTERVAL_MS);
    };

    const updateDisk = (disk) => {
        const rows = document.querySelectorAll('[data-monitor-disk]');
        for (const row of rows) {
            if (row.dataset.monitorDisk !== disk.mountPoint) {
                continue;
            }
            const usage = row.querySelector('[data-disk-field="usage"]');
            const summary = row.querySelector('[data-disk-field="summary"]');
            const progress = row.querySelector('[data-disk-field="progress"]');
            if (usage) {
                usage.textContent = formatPercent(disk.usagePercent);
            }
            if (summary) {
                summary.textContent = `${formatBytes(disk.usedBytes)} / ${formatBytes(disk.totalBytes)}`;
            }
            if (progress) {
                progress.value = disk.usagePercent;
            }
        }
    };

    const applySnapshot = (data) => {
        setText('cpu-usage', formatPercent(data.cpu.usagePercent));
        setProgress('cpu-progress', data.cpu.usagePercent);
        setText('load-1', Number(data.cpu.load1).toFixed(2));
        setText('load-5', Number(data.cpu.load5).toFixed(2));
        setText('load-15', Number(data.cpu.load15).toFixed(2));

        if (data.raspberryPi) {
            setText('pi-model', data.raspberryPi.model);
            setText('pi-revision', data.raspberryPi.revision);
            setText('pi-serial', data.raspberryPi.serial);
            setText(
                'pi-temperature',
                data.raspberryPi.temperatureCelsius == null
                    ? 'Unavailable'
                    : `${Number(data.raspberryPi.temperatureCelsius).toFixed(1)} °C`);
            setText(
                'pi-frequency',
                data.raspberryPi.cpuFrequencyHz == null
                    ? 'Unavailable'
                    : `${Math.round(Number(data.raspberryPi.cpuFrequencyHz) / 1000000)} MHz`);

            if (data.raspberryPi.health) {
                const health = data.raspberryPi.health;
                const currentWarning =
                    health.underVoltage ||
                    health.frequencyCapped ||
                    health.throttled ||
                    health.softTemperatureLimit;
                const historicalWarning =
                    health.underVoltageOccurred ||
                    health.frequencyCappingOccurred ||
                    health.throttlingOccurred ||
                    health.softTemperatureLimitOccurred;
                setText('pi-health-current', currentWarning ? 'Warning' : 'Normal');
                setText(
                    'pi-health-history',
                    historicalWarning ? 'Events recorded' : 'No events recorded');
            }
        }

        setText('memory-usage', formatPercent(data.memory.usagePercent));
        setProgress('memory-progress', data.memory.usagePercent);
        setText('memory-summary', `${formatBytes(data.memory.usedBytes)} of ${formatBytes(data.memory.totalBytes)} used`);
        setText('memory-available', `${formatBytes(data.memory.availableBytes)} available`);

        const uptime = formatDuration(data.system.uptimeSeconds);
        setText('uptime-hero', uptime);
        setText('uptime-system', uptime);
        const lastSuccessfulUpdate = data.monitor?.lastSuccessfulUpdate ?? data.timestamp;
        setText('last-update', new Date(lastSuccessfulUpdate * 1000).toLocaleTimeString());

        data.disks.forEach(updateDisk);
    };

    const poll = async () => {
        pollTimer = null;

        try {
            const response = await fetch('/api/system', {
                headers: { 'Accept': 'application/json' },
                cache: 'no-store'
            });
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }

            const data = await response.json();
            applySnapshot(data);
            reconnectAttempts = 0;
            pollingStopped = false;

            if (data.monitor?.healthy === false) {
                setConnectionState('stale', data.monitor);
            } else {
                setConnectionState('live');
            }

            schedulePoll();
        } catch (_) {
            if (reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
                pollingStopped = true;
                setConnectionState('offline');
                return;
            }

            reconnectAttempts += 1;
            setConnectionState('reconnecting');
            schedulePoll();
        }
    };

    const gpioPanel = document.querySelector('[data-gpio-panel]');
    const gpioSummary = document.querySelector('[data-gpio-summary]');
    const gpioStatus = document.querySelector('[data-gpio-status]');
    const gpioChips = document.querySelector('[data-gpio-chips]');
    const gpioFilters = document.querySelector('[data-gpio-filters]');
    let gpioFilter = 'active';
    let gpioData = null;

    const setGpioText = (element, value) => {
        if (element) {
            element.textContent = value;
        }
    };

    const gpioFlags = (line) => {
        const flags = [];
        if (line.activeLow) {
            flags.push('active-low');
        }
        if (line.drive && line.drive !== 'push-pull') {
            flags.push(line.drive);
        }
        return flags.length > 0 ? flags.join(', ') : '—';
    };

    const createGpioLine = (line) => {
        const row = document.createElement('tr');

        const offset = document.createElement('td');
        offset.className = 'gpio-offset';
        offset.textContent = String(line.offset);

        const name = document.createElement('td');
        const nameValue = document.createElement('strong');
        nameValue.textContent = line.name || 'Unnamed';
        name.appendChild(nameValue);

        const direction = document.createElement('td');
        const directionBadge = document.createElement('span');
        directionBadge.className = `gpio-badge gpio-direction gpio-direction-${line.direction || 'unknown'}`;
        directionBadge.textContent = line.direction || 'unknown';
        direction.appendChild(directionBadge);

        const functionCell = document.createElement('td');
        functionCell.textContent = line.function || 'GPIO';
        if (line.alternateFunction) {
            functionCell.className = 'gpio-function-active';
        }

        const consumer = document.createElement('td');
        consumer.textContent = line.consumer || '—';
        if (!line.consumer) {
            consumer.className = 'muted';
        }

        const flags = document.createElement('td');
        flags.textContent = gpioFlags(line);
        if (flags.textContent === '—') {
            flags.className = 'muted';
        }

        const state = document.createElement('td');
        const stateBadge = document.createElement('span');
        stateBadge.className = `gpio-badge ${line.used ? 'gpio-used' : 'gpio-free'}`;
        stateBadge.textContent = line.used ? 'Used' : 'Free';
        state.appendChild(stateBadge);

        row.append(offset, name, functionCell, direction, consumer, flags, state);
        return row;
    };

    const gpioLineMatchesFilter = (line) => {
        if (gpioFilter === 'active') {
            return line.used || line.alternateFunction;
        }
        if (gpioFilter === 'free') {
            return !line.used && !line.alternateFunction;
        }
        return true;
    };

    const updateGpioFilterButtons = () => {
        if (!gpioFilters) {
            return;
        }

        for (const button of gpioFilters.querySelectorAll('[data-gpio-filter]')) {
            const active = button.dataset.gpioFilter === gpioFilter;
            button.classList.toggle('is-active', active);
            button.setAttribute('aria-pressed', active ? 'true' : 'false');
        }
    };

    const renderGpio = (data) => {
        gpioData = data;

        if (!gpioPanel || !gpioChips) {
            return;
        }

        if (!data.available) {
            gpioPanel.hidden = true;
            return;
        }

        gpioPanel.hidden = false;

        const chips = Array.isArray(data.chips) ? data.chips : [];
        const totalLines = chips.reduce(
            (sum, chip) => sum + (Array.isArray(chip.lines) ? chip.lines.length : 0),
            0);
        const usedLines = chips.reduce(
            (sum, chip) => sum + (Array.isArray(chip.lines)
                ? chip.lines.filter((line) => line.used).length
                : 0),
            0);

        setGpioText(
            gpioSummary,
            `${chips.length} ${chips.length === 1 ? 'chip' : 'chips'} · ${totalLines} lines · ${usedLines} used`);

        if (data.monitor?.healthy === false) {
            setGpioText(
                gpioStatus,
                `Stale · ${formatAge(data.monitor.snapshotAgeMs ?? 0)} old`);
        } else {
            const updatedAt = data.monitor?.lastSuccessfulUpdate ?? data.timestamp;
            setGpioText(
                gpioStatus,
                updatedAt
                    ? `Updated ${new Date(updatedAt * 1000).toLocaleTimeString()}`
                    : 'GPIO inventory available');
        }

        const openChips = new Set(
            Array.from(gpioChips.querySelectorAll('details[open][data-gpio-chip]'))
                .map((details) => details.dataset.gpioChip));
        const hadRenderedChips =
            gpioChips.querySelector('details[data-gpio-chip]') !== null;

        gpioChips.replaceChildren();

        if (chips.length === 0) {
            const empty = document.createElement('p');
            empty.className = 'muted';
            empty.textContent = 'No GPIO chips detected.';
            gpioChips.appendChild(empty);
            return;
        }

        chips.forEach((chip, index) => {
            const details = document.createElement('details');
            details.className = 'gpio-chip';
            details.dataset.gpioChip = chip.name;
            details.open = hadRenderedChips
                ? openChips.has(chip.name)
                : index === 0;

            const lines = Array.isArray(chip.lines) ? chip.lines : [];
            const visibleLines = lines.filter(gpioLineMatchesFilter);
            const used = lines.filter((line) => line.used).length;

            const summary = document.createElement('summary');
            const identity = document.createElement('span');
            identity.className = 'gpio-chip-identity';

            const title = document.createElement('strong');
            title.textContent = chip.name;
            const label = document.createElement('span');
            label.className = 'muted';
            label.textContent = chip.label || 'Unlabelled GPIO chip';
            identity.append(title, label);

            const stats = document.createElement('span');
            stats.className = 'gpio-chip-stats';
            const lineCount = chip.lineCount ?? lines.length;
            stats.textContent = gpioFilter === 'all'
                ? `${lineCount} lines · ${used} used`
                : `${visibleLines.length} of ${lineCount} shown`;

            summary.append(identity, stats);

            const scroll = document.createElement('div');
            scroll.className = 'gpio-table-scroll';

            const table = document.createElement('table');
            table.className = 'gpio-table';

            const head = document.createElement('thead');
            const headerRow = document.createElement('tr');
            for (const titleText of ['Line', 'Name', 'Function', 'Direction', 'Consumer', 'Flags', 'State']) {
                const cell = document.createElement('th');
                cell.scope = 'col';
                cell.textContent = titleText;
                headerRow.appendChild(cell);
            }
            head.appendChild(headerRow);

            const body = document.createElement('tbody');
            for (const line of visibleLines) {
                body.appendChild(createGpioLine(line));
            }

            if (visibleLines.length === 0) {
                const emptyRow = document.createElement('tr');
                emptyRow.className = 'gpio-empty-row';
                const emptyCell = document.createElement('td');
                emptyCell.colSpan = 7;
                emptyCell.textContent =
                    gpioFilter === 'active'
                        ? 'No active lines on this chip.'
                        : gpioFilter === 'free'
                            ? 'No free lines on this chip.'
                            : 'No GPIO lines reported.';
                emptyRow.appendChild(emptyCell);
                body.appendChild(emptyRow);
            }

            table.append(head, body);
            scroll.appendChild(table);
            details.append(summary, scroll);
            gpioChips.appendChild(details);
        });
    };

    if (gpioFilters) {
        gpioFilters.addEventListener('click', (event) => {
            const button = event.target.closest('[data-gpio-filter]');
            if (!button || !gpioFilters.contains(button)) {
                return;
            }

            const nextFilter = button.dataset.gpioFilter;
            if (!['all', 'active', 'free'].includes(nextFilter) ||
                nextFilter === gpioFilter) {
                return;
            }

            gpioFilter = nextFilter;
            updateGpioFilterButtons();
            if (gpioData) {
                renderGpio(gpioData);
            }
        });
        updateGpioFilterButtons();
    }

    const pollGpio = async () => {
        gpioPollTimer = null;

        if (!gpioPanel) {
            return;
        }

        try {
            const response = await fetch('/api/gpio', {
                headers: { 'Accept': 'application/json' },
                cache: 'no-store'
            });
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }

            const data = await response.json();
            renderGpio(data);
        } catch (_) {
            setGpioText(gpioStatus, 'GPIO inventory unavailable');
        }

        gpioPollTimer = window.setTimeout(pollGpio, GPIO_POLL_INTERVAL_MS);
    };

    const retry = document.querySelector('[data-monitor-retry]');
    if (retry) {
        retry.addEventListener('click', () => {
            reconnectAttempts = 0;
            pollingStopped = false;

            if (pollTimer !== null) {
                window.clearTimeout(pollTimer);
                pollTimer = null;
            }

            setConnectionState('connecting');
            poll();
        });
    }

    schedulePoll();
    pollGpio();
})();