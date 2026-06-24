#include <drogular/pwa_scripts.hpp>

namespace drogular {

std::string PwaScripts::serviceWorkerRegistration(
    const std::string& path
) {
    return R"(<script>
if ("serviceWorker" in navigator) {
    window.addEventListener("load", () => {
        navigator.serviceWorker.register(")" + path + R"(");
    });
}
</script>)";
}

} // namespace drogular