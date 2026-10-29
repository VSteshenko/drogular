const SHELL_CACHE = "drogular-mini-store-shell-v21";
const STOREFRONT_CACHE = "drogular-mini-store-storefront-v1";
const OWNED_CACHES = [SHELL_CACHE, STOREFRONT_CACHE];

const APP_SHELL = [
    "/",
    "/__offline",
    "/assets/manifest.webmanifest",
    "/assets/favicon.ico",
    "/assets/icon-192.png",
    "/assets/icon-512.png",
    "/assets/banner.png",
    "/assets/storefront.css",
    "/assets/staff.css",
    "/assets/theme.css",
    "/assets/theme.js",
    "/assets/navigation.js",
    "/__drogular/assets/ui.css",
    "/__drogular/assets/interactions.js"
];

const isStorefrontReadPath = pathname =>
    pathname === "/" ||
    pathname === "/catalog" ||
    /^\/products\/\d+$/.test(pathname);

const isStorefrontFragmentPath = pathname =>
    pathname === "/fragments/storefront" ||
    pathname === "/fragments/storefront/catalog" ||
    /^\/fragments\/storefront\/products\/\d+$/.test(pathname);

const isCacheableStorefrontRequest = request => {
    if (request.method !== "GET") {
        return false;
    }

    const url = new URL(request.url);
    if (url.origin !== self.location.origin) {
        return false;
    }

    return isStorefrontReadPath(url.pathname) ||
        isStorefrontFragmentPath(url.pathname);
};

const cacheSuccessfulResponse = async (request, response) => {
    if (!response || !response.ok) {
        return response;
    }

    const cache = await caches.open(STOREFRONT_CACHE);
    await cache.put(request, response.clone());
    return response;
};

const networkFirstStorefront = async request => {
    try {
        const response = await fetch(request);
        return await cacheSuccessfulResponse(request, response);
    } catch (error) {
        const cached = await caches.match(request);
        if (cached) {
            return cached;
        }
        throw error;
    }
};

self.addEventListener("install", event => {
    event.waitUntil(
        caches.open(SHELL_CACHE)
            .then(cache => cache.addAll(APP_SHELL))
            .then(() => self.skipWaiting())
    );
});

self.addEventListener("activate", event => {
    event.waitUntil(
        caches.keys()
            .then(cacheNames => Promise.all(
                cacheNames
                    .filter(cacheName =>
                        cacheName.startsWith("drogular-mini-store-") &&
                        !OWNED_CACHES.includes(cacheName)
                    )
                    .map(cacheName => caches.delete(cacheName))
            ))
            .then(() => clients.claim())
    );
});

self.addEventListener("fetch", event => {
    const request = event.request;

    if (request.method !== "GET") {
        return;
    }

    if (isCacheableStorefrontRequest(request)) {
        event.respondWith(
            networkFirstStorefront(request).catch(async () => {
                if (request.mode === "navigate") {
                    return caches.match("/__offline");
                }

                // Fragment navigation has no safe synthetic representation.
                // Return an explicit offline response rather than replaying a
                // mutation-capable page or manufacturing stale application state.
                return new Response("", {
                    status: 503,
                    statusText: "Offline"
                });
            })
        );
        return;
    }

    if (request.mode === "navigate") {
        event.respondWith(
            fetch(request).catch(() => caches.match("/__offline"))
        );
        return;
    }

    event.respondWith(
        caches.match(request)
            .then(cached => cached || fetch(request))
    );
});