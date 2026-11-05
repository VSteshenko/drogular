'use strict';

const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

const workerPath = path.resolve(__dirname, '../../examples/portal_demo/public/service-worker.js');
const script = fs.readFileSync(workerPath, 'utf8');
const callbacks = new Map();
const stored = new Map();
const deleted = [];
const shellResources = [];
const cacheName = 'drogular-portal-demo-v13';
const localizationCacheName = 'drogular-localization-v1';
const otherCacheName = 'another-app-v4';

const cache = name => ({
    addAll: async urls => {
        for (const url of urls) {
            shellResources.push(url);
            stored.get(name).set(new URL(url, 'http://localhost:8083/').href, { url });
        }
    },
    match: async request => stored.get(name).get(new URL(
        typeof request === 'string' ? request : request.url,
        'http://localhost:8083/'
    ).href),
});

for (const name of ['drogular-portal-demo-v12', localizationCacheName, otherCacheName]) {
    stored.set(name, new Map());
}
stored.get(localizationCacheName).set('http://localhost:8083/assets/localization/en', { locale: 'en' });
stored.get(otherCacheName).set('http://localhost:8083/other', { body: 'preserved' });

const caches = {
    keys: async () => [...stored.keys()],
    open: async name => {
        if (!stored.has(name)) stored.set(name, new Map());
        return cache(name);
    },
    delete: async name => {
        deleted.push(name);
        return stored.delete(name);
    },
    match: async request => {
        for (const name of stored.keys()) {
            const result = await cache(name).match(request);
            if (result) return result;
        }
        return undefined;
    },
};

let skipWaitingCalled = false;
let claimCalled = false;
const context = {
    caches,
    clients: { claim: async () => { claimCalled = true; } },
    self: {
        addEventListener: (name, callback) => callbacks.set(name, callback),
        skipWaiting: async () => { skipWaitingCalled = true; },
    },
    URL,
    fetch: async () => { throw new Error('Network unavailable'); },
};
vm.runInNewContext(script, context, { filename: workerPath });

const lifecycle = async name => {
    let task;
    callbacks.get(name)({ waitUntil: promise => { task = promise; } });
    assert.ok(task, `${name} should call waitUntil`);
    await task;
};

(async () => {
    await lifecycle('install');
    assert.equal(skipWaitingCalled, true);
    assert.ok(shellResources.includes('/__drogular/assets/localization.js'));
    assert.ok(stored.get(cacheName).has('http://localhost:8083/__drogular/assets/localization.js'));

    await lifecycle('activate');
    assert.equal(claimCalled, true);
    assert.deepEqual(deleted, ['drogular-portal-demo-v12']);
    assert.equal(stored.has(localizationCacheName), true);
    assert.equal(stored.has(otherCacheName), true);
    assert.equal(stored.get(localizationCacheName).size, 1);

    let responsePromise;
    callbacks.get('fetch')({
        request: {
            method: 'GET',
            mode: 'same-origin',
            url: 'http://localhost:8083/__drogular/assets/localization.js',
        },
        respondWith: promise => { responsePromise = promise; },
    });
    assert.ok(responsePromise);
    assert.equal((await responsePromise).url, '/__drogular/assets/localization.js');
    console.log('PortalDemo service worker cache isolation and offline shell: OK');
})().catch(error => { console.error(error); process.exitCode = 1; });
