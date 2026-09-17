const CACHE_NAME = "drogular-portal-demo-v10";

const APP_SHELL = [
    "/",
    "/login",
    "/dashboard",
    "/__offline",
    "/assets/manifest.webmanifest",
    "/service-worker.js",
    "/assets/favicon.ico",
    "/__drogular/assets/ui.css",
    "/__drogular/assets/ui.js",
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
            .then(cacheNames => {
                return Promise.all(
                    cacheNames
                        .filter(cacheName => cacheName !== CACHE_NAME)
                        .map(cacheName => caches.delete(cacheName))
                );
            })
            .then(() => clients.claim())
    );
});

self.addEventListener("fetch", event => {
    if (event.request.method !== "GET") {
        return;
    }

    const url = new URL(event.request.url);
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