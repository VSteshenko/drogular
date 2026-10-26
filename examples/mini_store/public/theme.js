(() => {
    const storageKey = "drogular.mini-store.theme";
    const themes = new Set(["modern", "dark", "warm"]);
    const root = document.documentElement;
    const readTheme = () => {
        try { const value = localStorage.getItem(storageKey); return themes.has(value) ? value : "modern"; }
        catch (_) { return "modern"; }
    };
    const applyTheme = (theme, persist) => {
        const selected = themes.has(theme) ? theme : "modern";
        root.dataset.storeTheme = selected;
        root.dataset.dgTheme = selected === "dark" ? "dark" : "light";
        document.querySelectorAll("[data-theme-select]").forEach(select => { select.value = selected; });
        if (persist) { try { localStorage.setItem(storageKey, selected); } catch (_) {} }
    };

    applyTheme(readTheme(), false);
    document.addEventListener("change", event => {
        const select = event.target.closest("[data-theme-select]");
        if (select) applyTheme(select.value, true);
    });
    document.addEventListener("dg:after-replace", () => applyTheme(root.dataset.storeTheme || readTheme(), false));
    document.addEventListener("DOMContentLoaded", () => applyTheme(root.dataset.storeTheme || readTheme(), false));
})();