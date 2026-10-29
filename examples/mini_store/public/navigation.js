(() => {
    const syncTitle = (target) => {
        const page = target.querySelector('[data-mini-store-page-title]');
        if (page) {
            document.title = page.getAttribute('data-mini-store-page-title') || document.title;
        }
    };

    const syncStaffNavigation = () => {
        const path = window.location.pathname;
        document.querySelectorAll('.mini-store-staff-nav-item[data-staff-path]').forEach((item) => {
            const itemPath = item.getAttribute('data-staff-path');
            const active = itemPath === '/staff'
                ? path === '/staff'
                : path === itemPath || path.startsWith(`${itemPath}/`);
            item.classList.toggle('is-active', active);
            if (active) item.setAttribute('aria-current', 'page');
            else item.removeAttribute('aria-current');
        });
    };

    document.addEventListener('dg:after-replace', (event) => {
        if (!(event.target instanceof Element)) return;
        if (!event.target.matches('[data-storefront-content], [data-staff-content]')) return;
        syncTitle(event.target);
        syncStaffNavigation();
    });

    window.addEventListener('popstate', syncStaffNavigation);
    syncStaffNavigation();
})();