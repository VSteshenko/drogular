(() => {
    const syncPrimaryNavigation = () => {
        const path = window.location.pathname;
        document.querySelectorAll('.dg-nav > a.dg-nav-item').forEach((link) => {
            const active = link.getAttribute('href') === path;
            link.classList.toggle('is-active', active);
            if (active) {
                link.setAttribute('aria-current', 'page');
            } else {
                link.removeAttribute('aria-current');
            }
        });
    };

    document.addEventListener('dg:after-replace', (event) => {
        if (!(event.target instanceof Element) ||
            !event.target.matches('[data-portal-content]')) {
            return;
        }
        const page = event.target.querySelector('[data-portal-page-title]');
        if (page) {
            document.title = page.getAttribute('data-portal-page-title') || document.title;
        }
        syncPrimaryNavigation();
    });
})();