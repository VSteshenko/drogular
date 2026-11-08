#include "home_page.hpp"
#include "routes/jobs_demo_routes.hpp"
#include "startup/jobs_demo_startup.hpp"

#include <drogular/app.hpp>
#include <drogular/job_runtime.hpp>
#include <drogular/static_file_cache_profile.hpp>

#include <exception>
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    JobsDemoOptions options;
    try {
        options = JobsDemoStartup::parse(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n\n";
        JobsDemoStartup::printHelp(std::cerr);
        return 1;
    }
    if (options.showHelp) {
        JobsDemoStartup::printHelp();
        return 0;
    }

    drogular::App app;
    app.templateRoot("examples/jobs_demo/templates")
       .templateCache(false)
       .profile(drogular::ApplicationProfile::Development)
       .staticFiles("/assets", "examples/jobs_demo/public")
       .staticFileCacheProfile(drogular::StaticFileCacheProfile::Development)
       .page<JobsDemoHomePage>("/");

    auto jobs = std::make_shared<drogular::JobRuntime>(
        2, 32, &app.runtimeDiagnostics());
    jobsdemo::installRoutes(jobs, app.runtimeDiagnostics());
    JobsDemoStartup::printBanner(options);
    app.run(options.port);
    jobs->shutdown();
}