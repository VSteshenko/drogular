(() => {
    const syncTitle = (target) => {
        const page = target.querySelector('[data-mini-store-page-title]');
        if (page) {
            document.title = page.getAttribute('data-mini-store-page-title') || document.title;
        }
    };

    document.addEventListener('dg:after-replace', (event) => {
        if (!(event.target instanceof Element)) return;
        if (!event.target.matches('[data-storefront-content], [data-staff-content]')) return;
        syncTitle(event.target);
    });
})();