(() => {
    const sectionForPath = (path) => {
        if (path === '/dashboard') return '/dashboard';
        if (path === '/users' || path.startsWith('/users/')) return '/users';
        if (path === '/projects' || path.startsWith('/projects/')) return '/projects';
        if (path === '/departments' || path.startsWith('/departments/')) return '/departments';
        return '';
    };

    const syncNavigation = () => {
        const path = window.location.pathname;
        const primarySection = sectionForPath(path);

        document.querySelectorAll('.dg-nav > a.dg-nav-item').forEach((link) => {
            const active = link.getAttribute('href') === primarySection;
            link.classList.toggle('is-active', active);
            if (active) {
                link.setAttribute('aria-current', 'page');
            } else {
                link.removeAttribute('aria-current');
            }
        });

        const adminSection =
            path === '/admin' ||
            path === '/roles' || path.startsWith('/roles/') ||
            path === '/project-types' || path.startsWith('/project-types/');

        const adminGroup = document.querySelector('.dg-nav-group');
        if (adminGroup) {
            adminGroup.classList.toggle('is-active', adminSection);
        }

        document.querySelectorAll('.dg-nav-group > a.dg-nav-item, .dg-nav-subitem').forEach((link) => {
            const href = link.getAttribute('href');
            const active =
                href === '/admin'
                    ? path === '/admin'
                    : href === '/roles'
                        ? path === '/roles' || path.startsWith('/roles/')
                        : href === '/project-types'
                            ? path === '/project-types' || path.startsWith('/project-types/')
                            : false;
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
        syncNavigation();
    });
})();
