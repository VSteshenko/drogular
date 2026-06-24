const CACHE_NAME = "drogular-todo-pwa-v6";

const APP_SHELL = [
    "/",
    "/assets/manifest.webmanifest",
    "/service-worker.js",
    "/__offline"
];

const STATIC_ASSETS = [
    "/assets/favicon.ico",
    "/assets/todo-icon-192.png",
    "/assets/todo-icon-512.png"
];

self.addEventListener("install", event => {
    event.waitUntil(
        caches.open(CACHE_NAME)
            .then(cache => cache.addAll([
                ...APP_SHELL,
                ...STATIC_ASSETS
            ]))
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