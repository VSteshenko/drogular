(() => {
    const translations = (() => {
        const element = document.getElementById('system-monitor-i18n');
        if (!element) return {};
        try {
            return JSON.parse(element.textContent || '{}');
        } catch (_) {
            return {};
        }
    })();
    const tr = (key, fallback = key) => translations[key] ?? fallback;

    const REFRESH_INTERVAL_MS = 30000;
    const hasPhysicalHeader = Boolean(document.querySelector('[data-board-header-map]'));

    const staticPins = hasPhysicalHeader ? new Map([
        [1, { label: '3V3', kind: 'power' }],
        [2, { label: '5V', kind: 'power' }],
        [4, { label: '5V', kind: 'power' }],
        [6, { label: 'GND', kind: 'ground' }],
        [9, { label: 'GND', kind: 'ground' }],
        [14, { label: 'GND', kind: 'ground' }],
        [17, { label: '3V3', kind: 'power' }],
        [20, { label: 'GND', kind: 'ground' }],
        [25, { label: 'GND', kind: 'ground' }],
        [30, { label: 'GND', kind: 'ground' }],
        [34, { label: 'GND', kind: 'ground' }],
        [39, { label: 'GND', kind: 'ground' }]
    ]) : new Map();

    const summaryValue = (name) => document.querySelector(`[data-board-summary-value="${name}"]`);
    const summaryDetail = (name) => document.querySelector(`[data-board-summary-detail="${name}"]`);
    const setText = (selector, value) => {
        const element = document.querySelector(selector);
        if (element) element.textContent = value;
    };

    const fetchJson = async (url) => {
        const response = await fetch(url, {
            headers: { 'Accept': 'application/json' },
            cache: 'no-store'
        });
        if (!response.ok) throw new Error(`${url}: HTTP ${response.status}`);
        return response.json();
    };

    const exposureLabel = (value) => {
        if (value === 'header') return '40-pin header';
        if (value === 'onboard') return 'onboard';
        if (value === 'internal') return 'internal';
        if (value === 'mixed') return 'mixed';
        return 'unknown';
    };

    const healthyLabel = (data) => {
        if (!data?.available) return tr('status.unavailable', 'Unavailable');
        if (data.monitor?.healthy === false) return tr('status.stale', 'Stale');
        return tr('status.healthy', 'Healthy');
    };

    const renderConnectionStatus = (system) => {
        const status = document.querySelector('[data-monitor-status]');
        const label = document.querySelector('[data-monitor-status-label]');
        const detail = document.querySelector('[data-monitor-status-detail]');
        if (!status || !label) return;

        const stale = system?.monitor?.healthy === false;
        const unavailable = !system;
        status.classList.toggle('status-stale', stale);
        status.classList.toggle('status-offline', unavailable);
        if (unavailable) {
            label.textContent = tr('status.offline', 'Offline');
        } else if (stale) {
            label.textContent = tr('status.stale', 'Stale');
        } else {
            label.textContent = tr('status.live', 'Live');
        }
        if (detail) {
            detail.hidden = !stale;
            detail.textContent = stale ? tr('client.hardware_overview_stale') : '';
        }
    };

    const gpioHeaderLines = (gpio) => {
        const lines = [];
        for (const chip of Array.isArray(gpio?.chips) ? gpio.chips : []) {
            for (const line of Array.isArray(chip.lines) ? chip.lines : []) {
                if (Number.isInteger(line.physicalHeaderPin)) {
                    lines.push({ ...line, chip: chip.name });
                }
            }
        }
        return lines;
    };

    const interfacePins = (i2c, spi, uart) => {
        const byPhysicalPin = new Map();
        const add = (pin, label, detail) => {
            if (!Number.isInteger(pin?.physicalHeaderPin)) return;
            const list = byPhysicalPin.get(pin.physicalHeaderPin) || [];
            list.push({ label, detail });
            byPhysicalPin.set(pin.physicalHeaderPin, list);
        };

        for (const bus of Array.isArray(i2c?.buses) ? i2c.buses : []) {
            for (const pin of Array.isArray(bus.gpioPins) ? bus.gpioPins : []) {
                add(pin, `I²C${bus.number} ${String(pin.role || '').toUpperCase()}`, pin.function);
            }
        }
        for (const bus of Array.isArray(spi?.buses) ? spi.buses : []) {
            for (const pin of Array.isArray(bus.gpioPins) ? bus.gpioPins : []) {
                add(pin, `SPI${bus.number} ${String(pin.role || '').toUpperCase()}`, pin.function || pin.consumer);
            }
        }
        for (const group of Array.isArray(uart?.gpioGroups) ? uart.gpioGroups : []) {
            for (const pin of Array.isArray(group.pins) ? group.pins : []) {
                add(pin, `UART${group.controller} ${String(pin.role || '').toUpperCase()}`, pin.function);
            }
        }
        return byPhysicalPin;
    };

    const renderHeader = (system, gpio, i2c, spi, uart) => {
        const map = document.querySelector('[data-board-header-map]');
        const status = document.querySelector('[data-board-header-status]');
        if (!map) return;

        const lines = gpioHeaderLines(gpio);
        const hasHeaderMetadata = lines.length > 0;
        if (!system?.raspberryPi || !hasHeaderMetadata) {
            map.replaceChildren();
            const empty = document.createElement('p');
            empty.className = 'muted';
            empty.textContent = system?.raspberryPi
                ? tr('client.physical_metadata_unavailable')
                : tr('client.header_not_available');
            map.appendChild(empty);
            if (status) status.textContent = tr('client.header_metadata_unavailable');
            return;
        }

        const gpioByPin = new Map(lines.map((line) => [line.physicalHeaderPin, line]));
        const rolesByPin = interfacePins(i2c, spi, uart);
        map.replaceChildren();

        for (let row = 0; row < 20; ++row) {
            const rowElement = document.createElement('div');
            rowElement.className = 'board-header-row';
            const leftPin = row * 2 + 1;
            const rightPin = leftPin + 1;
            rowElement.appendChild(renderPhysicalPin(leftPin, gpioByPin.get(leftPin), rolesByPin.get(leftPin)));
            rowElement.appendChild(renderPhysicalPin(rightPin, gpioByPin.get(rightPin), rolesByPin.get(rightPin)));
            map.appendChild(rowElement);
        }

        const activePins = [...rolesByPin.keys()].length;
        if (status) status.textContent = `${lines.length} GPIO pins · ${activePins} interface pins`;
    };

    const renderPhysicalPin = (number, gpio, roles) => {
        const pin = document.createElement('article');
        const staticInfo = staticPins.get(number);
        const hasRoles = Array.isArray(roles) && roles.length > 0;
        const kind = staticInfo?.kind || 'gpio';
        pin.className = `board-pin board-pin-${kind}${hasRoles ? ' board-pin-interface' : ''}`;
        pin.dataset.physicalPin = String(number);

        const numberElement = document.createElement('span');
        numberElement.className = 'board-pin-number';
        numberElement.textContent = String(number);
        pin.appendChild(numberElement);

        const body = document.createElement('div');
        body.className = 'board-pin-body';
        const label = document.createElement('strong');
        label.textContent = staticInfo?.label || gpio?.name || `Pin ${number}`;
        body.appendChild(label);

        if (gpio) {
            const detail = document.createElement('span');
            detail.className = 'muted board-pin-detail';
            const state = gpio.used ? 'used' : (gpio.alternateFunction ? 'muxed' : 'free');
            detail.textContent = `${gpio.function || gpio.direction || 'GPIO'} · ${state}`;
            body.appendChild(detail);
        }

        if (hasRoles) {
            const tags = document.createElement('div');
            tags.className = 'board-pin-tags';
            for (const role of roles) {
                const tag = document.createElement('span');
                tag.className = 'board-pin-tag';
                tag.textContent = role.label;
                if (role.detail) tag.title = role.detail;
                tags.appendChild(tag);
            }
            body.appendChild(tags);
        }

        pin.appendChild(body);
        return pin;
    };

    const renderSummary = (gpio, i2c, spi, uart) => {
        const chips = Array.isArray(gpio?.chips) ? gpio.chips : [];
        const gpioLines = chips.reduce((count, chip) => count + (Array.isArray(chip.lines) ? chip.lines.length : 0), 0);
        const headerLines = gpioHeaderLines(gpio).length;
        if (summaryValue('gpio')) summaryValue('gpio').textContent = gpio?.available
            ? (hasPhysicalHeader ? `${headerLines} ${tr("client.header_gpio")}` : `${gpioLines} ${tr("client.gpio_lines")}`)
            : tr('status.unavailable', 'Unavailable');
        if (summaryDetail('gpio')) summaryDetail('gpio').textContent = gpio?.available ? `${chips.length} ${tr("client.chips")} · ${gpioLines} ${tr("client.lines")} · ${healthyLabel(gpio)}` : `GPIO ${tr('client.inventory_unavailable')}`;

        const i2cBuses = Array.isArray(i2c?.buses) ? i2c.buses : [];
        const i2cDevices = i2cBuses.reduce((count, bus) => count + (Array.isArray(bus.devices) ? bus.devices.length : 0), 0);
        if (summaryValue('i2c')) summaryValue('i2c').textContent = i2c?.available ? `${i2cBuses.length} ${tr("client.buses")}` : tr('status.unavailable', 'Unavailable');
        if (summaryDetail('i2c')) summaryDetail('i2c').textContent = i2c?.available ? `${i2cDevices} ${tr("client.detected_devices")} · ${healthyLabel(i2c)}` : `I²C ${tr('client.inventory_unavailable')}`;

        const spiBuses = Array.isArray(spi?.buses) ? spi.buses : [];
        const spiDevices = spiBuses.reduce((count, bus) => count + (Array.isArray(bus.devices) ? bus.devices.length : 0), 0);
        if (summaryValue('spi')) summaryValue('spi').textContent = spi?.available ? `${spiBuses.length} ${tr("client.buses")}` : tr('status.unavailable', 'Unavailable');
        if (summaryDetail('spi')) summaryDetail('spi').textContent = spi?.available ? `${spiDevices} ${tr("client.spidev_nodes")} · ${healthyLabel(spi)}` : `SPI ${tr('client.inventory_unavailable')}`;

        const uartDevices = Array.isArray(uart?.devices) ? uart.devices : [];
        const uartGroups = Array.isArray(uart?.gpioGroups) ? uart.gpioGroups : [];
        if (summaryValue('uart')) summaryValue('uart').textContent = uart?.available ? `${uartDevices.length} ${tr("client.devices")}` : tr('status.unavailable', 'Unavailable');
        if (summaryDetail('uart')) summaryDetail('uart').textContent = uart?.available ? `${uartGroups.length} ${tr("client.pinmux_groups")} · ${healthyLabel(uart)}` : `UART ${tr('client.inventory_unavailable')}`;
    };

    const makeInterfaceCard = (title, status, exposure, details, badges = []) => {
        const card = document.createElement('article');
        card.className = 'panel board-interface-card';
        const heading = document.createElement('div');
        heading.className = 'board-interface-heading';
        const titleElement = document.createElement('h3');
        titleElement.textContent = title;
        heading.appendChild(titleElement);
        const exposureBadge = document.createElement('span');
        exposureBadge.className = `board-exposure board-exposure-${exposure || 'unknown'}`;
        exposureBadge.textContent = exposureLabel(exposure);
        heading.appendChild(exposureBadge);
        card.appendChild(heading);

        const state = document.createElement('p');
        state.className = 'board-interface-state';
        state.textContent = status;
        card.appendChild(state);

        const detail = document.createElement('p');
        detail.className = 'muted';
        detail.textContent = details;
        card.appendChild(detail);

        if (badges.length > 0) {
            const badgeList = document.createElement('div');
            badgeList.className = 'board-interface-badges';
            for (const value of badges) {
                const badge = document.createElement('span');
                badge.textContent = value;
                badgeList.appendChild(badge);
            }
            card.appendChild(badgeList);
        }
        return card;
    };

    const pinBadge = (pin, role) => {
        const label = String(role || '').toUpperCase();
        if (hasPhysicalHeader && Number.isInteger(pin?.physicalHeaderPin)) {
            return `${label} · pin ${pin.physicalHeaderPin}`;
        }
        const gpioName = pin?.name || (Number.isInteger(pin?.offset) ? `GPIO${pin.offset}` : 'GPIO');
        const functionName = pin?.function && pin.function !== gpioName ? ` · ${pin.function}` : '';
        return `${label} · ${gpioName}${functionName}`;
    };

    const renderInterfaces = (gpio, i2c, spi, uart) => {
        const target = document.querySelector('[data-board-interface-list]');
        if (!target) return;
        target.replaceChildren();

        const chips = Array.isArray(gpio?.chips) ? gpio.chips : [];
        const allLines = chips.flatMap((chip) =>
            (Array.isArray(chip.lines) ? chip.lines : []).map((line) => ({ ...line, chip: chip.name })));
        const headerLines = gpioHeaderLines(gpio);
        const visibleLines = hasPhysicalHeader ? headerLines : allLines;
        const activeLines = visibleLines.filter((line) => line.used || line.alternateFunction);
        target.appendChild(makeInterfaceCard(
            'GPIO',
            gpio?.available ? healthyLabel(gpio) : tr('status.unavailable', 'Unavailable'),
            hasPhysicalHeader && headerLines.length > 0 ? 'header' : 'unknown',
            gpio?.available
                ? `${activeLines.length} active or muxed ${hasPhysicalHeader ? 'header ' : ''}lines`
                : tr('client.gpio_service_unavailable'),
            activeLines.slice(0, 8).map((line) =>
                hasPhysicalHeader && Number.isInteger(line.physicalHeaderPin)
                    ? `${line.name} · pin ${line.physicalHeaderPin}`
                    : `${line.name}${line.function ? ` · ${line.function}` : ''}`)));

        for (const bus of Array.isArray(i2c?.buses) ? i2c.buses : []) {
            const devices = Array.isArray(bus.devices) ? bus.devices : [];
            const pins = Array.isArray(bus.gpioPins) ? bus.gpioPins : [];
            target.appendChild(makeInterfaceCard(
                `I²C ${bus.number}`,
                bus.scanned ? `${devices.length} detected ${devices.length === 1 ? 'device' : 'devices'}` : 'Inventory only',
                bus.exposure,
                bus.description || bus.name || 'I²C bus',
                [
                    ...pins.map((pin) => pinBadge(pin, pin.role)),
                    ...devices.map((device) => device.addressHex || `0x${Number(device.address).toString(16)}`)
                ]));
        }

        for (const bus of Array.isArray(spi?.buses) ? spi.buses : []) {
            const devices = Array.isArray(bus.devices) ? bus.devices : [];
            const pins = Array.isArray(bus.gpioPins) ? bus.gpioPins : [];
            target.appendChild(makeInterfaceCard(
                `SPI ${bus.number}`,
                `${devices.length} ${devices.length === 1 ? 'device node' : 'device nodes'}`,
                bus.exposure,
                devices.map((device) => device.path).join(' · ') || tr('client.no_spi'),
                pins.map((pin) => pinBadge(pin, pin.role))));
        }

        const aliases = [];
        for (const device of Array.isArray(uart?.devices) ? uart.devices : []) {
            aliases.push(device.path || device.name);
            for (const alias of Array.isArray(device.aliases) ? device.aliases : []) aliases.push(alias);
        }
        for (const group of Array.isArray(uart?.gpioGroups) ? uart.gpioGroups : []) {
            const pins = Array.isArray(group.pins) ? group.pins : [];
            target.appendChild(makeInterfaceCard(
                `UART ${group.controller}`,
                group.exposure === 'header' ? tr('client.available_on_header') : tr('client.detected_pinmux'),
                group.exposure,
                aliases.length > 0 ? `Linux serial: ${aliases.join(' · ')}` : tr('client.no_linux_serial'),
                pins.map((pin) => pinBadge(pin, pin.role))));
        }
    };

    const refresh = async () => {
        const results = await Promise.allSettled([
            fetchJson('/api/system'),
            fetchJson('/api/gpio'),
            fetchJson('/api/i2c'),
            fetchJson('/api/spi'),
            fetchJson('/api/uart')
        ]);
        const value = (index) => results[index].status === 'fulfilled' ? results[index].value : null;
        const system = value(0);
        const gpio = value(1);
        const i2c = value(2);
        const spi = value(3);
        const uart = value(4);

        renderConnectionStatus(system);

        if (system?.raspberryPi) {
            setText('[data-board-model]', system.raspberryPi.model);
            setText('[data-board-model-detail]', system.raspberryPi.model);
            setText('[data-board-revision]', system.raspberryPi.revision || tr('status.unavailable', 'Unavailable'));
            setText('[data-board-serial]', system.raspberryPi.serial || tr('status.unavailable', 'Unavailable'));
        }

        const succeeded = results.filter((result) => result.status === 'fulfilled').length;
        const availableInventories = [gpio, i2c, spi, uart]
            .filter((inventory) => inventory?.available).length;
        setText(
            '[data-board-inventory-status]',
            `${succeeded}/5 ${tr("client.data_sources_reachable")} · ${availableInventories}/4 ${tr("client.hardware_inventories_available")}`);
        renderSummary(gpio, i2c, spi, uart);
        renderHeader(system, gpio, i2c, spi, uart);
        renderInterfaces(gpio, i2c, spi, uart);
    };

    const poll = async () => {
        try {
            await refresh();
        } finally {
            window.setTimeout(poll, REFRESH_INTERVAL_MS);
        }
    };

    poll();
})();
