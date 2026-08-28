(() => {
    const POLL_INTERVAL_MS = 2000;
    const MAX_RECONNECT_ATTEMPTS = 3;

    let reconnectAttempts = 0;
    let pollTimer = null;
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
})();