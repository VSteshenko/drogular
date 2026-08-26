(() => {
    const POLL_INTERVAL_MS = 2000;

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

    const setConnectionState = (live) => {
        const status = document.querySelector('[data-monitor-status]');
        if (!status) {
            return;
        }
        status.classList.toggle('status-offline', !live);
        status.querySelector('[data-monitor-status-label]').textContent = live ? 'Live' : 'Reconnecting';
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

        setText('memory-usage', formatPercent(data.memory.usagePercent));
        setProgress('memory-progress', data.memory.usagePercent);
        setText('memory-summary', `${formatBytes(data.memory.usedBytes)} of ${formatBytes(data.memory.totalBytes)} used`);
        setText('memory-available', `${formatBytes(data.memory.availableBytes)} available`);

        const uptime = formatDuration(data.system.uptimeSeconds);
        setText('uptime-hero', uptime);
        setText('uptime-system', uptime);
        setText('last-update', new Date(data.timestamp * 1000).toLocaleTimeString());

        data.disks.forEach(updateDisk);
    };

    const poll = async () => {
        try {
            const response = await fetch('/api/system', {
                headers: { 'Accept': 'application/json' },
                cache: 'no-store'
            });
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }
            applySnapshot(await response.json());
            setConnectionState(true);
        } catch (_) {
            setConnectionState(false);
        } finally {
            window.setTimeout(poll, POLL_INTERVAL_MS);
        }
    };

    window.setTimeout(poll, POLL_INTERVAL_MS);
})();