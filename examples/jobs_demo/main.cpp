#include "home_page.hpp"
#include "routes/jobs_demo_routes.hpp"
#include "startup/jobs_demo_startup.hpp"

#include <drogular/app.hpp>
#include <drogular/job_runtime.hpp>
#include <drogular/job_registry.hpp>
#include <drogular/developer_tools/job_registry_contributor.hpp>
#include <drogular/static_file_cache_profile.hpp>

#include <chrono>
#include <string>
#include <thread>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>
#include <cstdint>

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
       .jobClient()
       .profile(drogular::ApplicationProfile::Development)
       .staticFiles("/assets", "examples/jobs_demo/public")
       .staticFileCacheProfile(drogular::StaticFileCacheProfile::Development)
       .page<JobsDemoHomePage>("/");

    auto jobs = std::make_shared<drogular::JobRuntime>(
        2, 32, &app.runtimeDiagnostics());
    auto registry = std::make_shared<drogular::JobRegistry>(*jobs);
    auto registerDemo = [&](std::string type, std::string title, bool fail) {
        drogular::JobDefinition definition;
        definition.id = std::move(type);
        definition.name = std::move(title);
        definition.description = fail ? "Demonstrate structured errors" :
            "Process batches with progress updates and cooperative cancellation";
        definition.parameters.push_back({
            "seconds",
            drogular::JobParameterType::Integer,
            false,
            "15",
            1,
            120
        });
        definition.parameters.push_back({
            "memoryKiB",
            drogular::JobParameterType::Integer,
            false,
            "0",
            0,
            2048,
            {"0", "512", "1024", "2048"}
        });
        registry->registerJob(
            std::move(definition),
            [fail](
                drogular::JobContext& ctx,
                const drogular::JobParameters& parameters
            ) {
                const int seconds = std::stoi(parameters.at("seconds"));
                const auto requestedKiB = static_cast<std::size_t>(
                    std::stoul(parameters.at("memoryKiB")));
                // Keep the allocation alive across the entire task; touching
                // pages ensures the operating system commits physical memory.
                std::vector<std::uint8_t> memory(requestedKiB * 1024, 0);
                for (std::size_t i = 0; i < memory.size(); i += 4096) {
                    memory[i] = static_cast<std::uint8_t>(i / 4096 % 255 + 1);
                }
                const auto allocationLabel = std::to_string(requestedKiB) + " KiB";
                for (int step = 1; step <= seconds * 5; ++step) {
                    ctx.throwIfCancellationRequested();
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    ctx.throwIfCancellationRequested();
                    ctx.reportProgress(
                        static_cast<unsigned>(step * 100 / (seconds * 5)),
                        "Processing batch " + std::to_string(step) + " of " +
                        std::to_string(seconds * 5) + " · allocated " + allocationLabel);
                }
                // Read allocated memory to keep its lifetime observable to
                // the optimizer. RAII also releases it on cancellation/failure.
                std::uint64_t checksum = 0;
                for (std::size_t i = 0; i < memory.size(); i += 4096) {
                    checksum += memory[i];
                }
                if (fail) {
                    throw drogular::JobFailure(
                        "DEMO_FAILURE",
                        "Intentional demo failure · allocated " + allocationLabel);
                }
                ctx.setResult("All batches processed successfully · allocated " +
                              allocationLabel + " · checksum " + std::to_string(checksum));
            }
        );
    };
    registerDemo("demo.batch", "Background operation", false);
    registerDemo("demo.failure", "Failure demonstration", true);
    app.developerToolsContributor(
        std::make_shared<drogular::JobRegistryContributor>(
            registry, "/api/jobs/registry/launch"
        )
    );
    jobsdemo::installRoutes(jobs, registry, app.runtimeDiagnostics());
    JobsDemoStartup::printBanner(options);

    app.run(options.port);
    jobs->shutdown();
}