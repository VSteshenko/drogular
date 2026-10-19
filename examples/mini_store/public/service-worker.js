const CACHE_NAME = "drogular-mini-store-v1";

const APP_SHELL = [
    "/",
    "/__offline",
    "/assets/manifest.webmanifest",
    "/assets/favicon.ico",
    "/assets/icon-192.png",
    "/assets/icon-512.png",
    "/assets/banner.png",
    "/assets/storefront.css",
    "/assets/navigation.js",
    "/__drogular/assets/ui.css",
    "/__drogular/assets/interactions.js"
];

self.addEventListener("install", event => {
    event.waitUntil(
        caches.open(CACHE_NAME)
            .then(cache => cache.addAll(APP_SHELL))
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

    // E.7.2 intentionally leaves fragment/read-model offline navigation to E.7.3.
    if (url.pathname.startsWith("/fragments/")) {
        return;
    }

    if (event.request.mode === "navigate") {
        event.respondWith(
            fetch(event.request)
                .catch(() => caches.match("/__offline"))
        );
        return;
    }

    event.respondWith(
        caches.match(event.request)
            .then(cached => cached || fetch(event.request))
    );
});