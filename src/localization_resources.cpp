#include <drogular/localization_resources.hpp>

namespace drogular::localization_resources {
namespace {

constexpr std::string_view Script = R"DROGULAR_JS((() => {
    let manifestUrl = null;
    let manifestValue = null;
    let activeLocale = null;
    const resources = new Map();

    const requireObject = (value, name) => {
        if (!value || typeof value !== 'object' || Array.isArray(value)) {
            throw new Error(`Invalid localization ${name}`);
        }
        return value;
    };

    const validateManifest = (value) => {
        const manifest = requireObject(value, 'manifest');
        if (typeof manifest.defaultLocale !== 'string' || !manifest.defaultLocale) {
            throw new Error('Localization manifest is missing defaultLocale');
        }
        requireObject(manifest.locales, 'manifest locales');
        if (!manifest.locales[manifest.defaultLocale]) {
            throw new Error('Localization manifest does not contain its default locale');
        }
        for (const [locale, descriptorValue] of Object.entries(manifest.locales)) {
            const descriptor = requireObject(descriptorValue, `locale descriptor: ${locale}`);
            if (typeof descriptor.url !== 'string' || !descriptor.url) {
                throw new Error(`Localization locale ${locale} is missing url`);
            }
            if (typeof descriptor.version !== 'string' || !descriptor.version) {
                throw new Error(`Localization locale ${locale} is missing version`);
            }
        }
        return manifest;
    };

    const fetchJson = async (url) => {
        const response = await fetch(url, { headers: { Accept: 'application/json' } });
        if (!response.ok) {
            throw new Error(`Localization request failed: ${response.status} ${url}`);
        }
        return response.json();
    };

    const loadManifest = async ({ force = false } = {}) => {
        if (!manifestUrl) {
            throw new Error('Localization runtime is not configured');
        }
        if (manifestValue && !force) return manifestValue;

        const next = validateManifest(await fetchJson(manifestUrl));
        if (manifestValue?.version !== next.version) {
            resources.clear();
        }
        manifestValue = next;
        return manifestValue;
    };

    const loadResource = async (locale) => {
        const manifest = await loadManifest();
        const descriptor = manifest.locales[locale];
        if (!descriptor) return null;

        const cached = resources.get(locale);
        if (cached?.version === descriptor.version) return cached;

        const resource = requireObject(
            await fetchJson(descriptor.url),
            `resource: ${locale}`
        );
        if (resource.locale !== locale) {
            throw new Error(`Localization resource locale mismatch: ${locale}`);
        }
        if (resource.version !== descriptor.version) {
            throw new Error(`Localization resource version mismatch: ${locale}`);
        }
        requireObject(resource.translations, `translations: ${locale}`);
        resources.set(locale, resource);
        return resource;
    };

    const setLocale = async (locale) => {
        const manifest = await loadManifest();
        const requested = manifest.locales[locale]
            ? locale
            : manifest.defaultLocale;

        await loadResource(manifest.defaultLocale);
        if (requested !== manifest.defaultLocale) {
            await loadResource(requested);
        }

        activeLocale = requested;
        document.documentElement.lang = requested;
        document.dispatchEvent(new CustomEvent('dg:locale-changed', {
            detail: { locale: requested },
        }));
        return requested;
    };

    const translate = (key) => {
        if (typeof key !== 'string' || !key) return key;
        if (!manifestValue) return key;

        const locale = activeLocale || manifestValue.defaultLocale;
        const requested = resources.get(locale)?.translations;
        if (requested && Object.prototype.hasOwnProperty.call(requested, key)) {
            return requested[key];
        }

        const fallback = resources.get(manifestValue.defaultLocale)?.translations;
        if (fallback && Object.prototype.hasOwnProperty.call(fallback, key)) {
            return fallback[key];
        }
        return key;
    };

    const configure = ({ manifestUrl: nextManifestUrl } = {}) => {
        if (typeof nextManifestUrl !== 'string' || !nextManifestUrl) {
            throw new Error('Localization manifestUrl must be a non-empty string');
        }
        if (manifestUrl !== nextManifestUrl) {
            manifestValue = null;
            activeLocale = null;
            resources.clear();
        }
        manifestUrl = nextManifestUrl;
        return api;
    };

    const ready = async (locale = document.documentElement.lang || '') => {
        const manifest = await loadManifest();
        return setLocale(locale || manifest.defaultLocale);
    };

    const api = Object.freeze({
        configure,
        loadManifest,
        loadResource,
        ready,
        setLocale,
        translate,
        locale: () => activeLocale,
        defaultLocale: () => manifestValue?.defaultLocale || null,
        manifest: () => manifestValue,
    });

    globalThis.drogularLocalization = api;
})();
)DROGULAR_JS";

} // namespace

std::string_view script() {
    return Script;
}

} // namespace drogular::localization_resources