const CACHE_NAME = "drogular-system-monitor-v3";

const OFFLINE_PAGE = "/__offline";

const STATIC_ASSETS = [
    OFFLINE_PAGE,
    "/assets/manifest.webmanifest",
    "/assets/app.css",
    "/assets/app.js",
    "/assets/board.js",
    "/assets/icons/favicon.svg",
    "/assets/icons/apple-touch-icon.png",
    "/assets/icons/icon-192.png",
    "/assets/icons/icon-512.png",
    "/assets/icons/icon-512-maskable.png",
    "/assets/screenshot-wide.png",
    "/assets/screenshot-mobile.png"
];

self.addEventListener("install", event => {
    event.waitUntil(
        caches.open(CACHE_NAME)
            .then(cache => cache.addAll(STATIC_ASSETS))
            .then(() => self.skipWaiting())
    );
});

self.addEventListener("activate", event => {
    event.waitUntil(
        caches.keys()
            .then(cacheNames => Promise.all(
                cacheNames
                    .filter(cacheName => cacheName !== CACHE_NAME)
                    .map(cacheName => caches.delete(cacheName))
            ))
            .then(() => clients.claim())
    );
});

self.addEventListener("fetch", event => {
    if (event.request.method !== "GET") {
        return;
    }

    const url = new URL(event.request.url);

    // Monitoring APIs must always reflect the network/backend state. Never
    // satisfy them from the service-worker cache.
    if (url.origin === self.location.origin && url.pathname.startsWith("/api/")) {
        event.respondWith(fetch(event.request));
        return;
    }

    // Rendered pages contain live snapshot data. If navigation cannot reach
    // the server, show the dedicated offline page rather than cached metrics.
    if (event.request.mode === "navigate") {
        event.respondWith(
            fetch(event.request)
                .catch(() => caches.match(OFFLINE_PAGE))
        );
        return;
    }

    if (url.origin === self.location.origin && url.pathname.startsWith("/assets/")) {
        event.respondWith(
            caches.match(event.request)
                .then(cached => cached || fetch(event.request))
        );
    }
});