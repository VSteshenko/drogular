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

std::string PwaScripts::inputPersistence(
    const std::string& inputId,
    const std::string& storageKey
) {
    return R"(<script>
(() => {
    const input = document.getElementById(")" + inputId + R"(");

    if (!input) {
        return;
    }

    const storageKey = ")" + storageKey + R"(";

    const savedValue = localStorage.getItem(storageKey);

    if (savedValue !== null) {
        input.value = savedValue;
    }

    input.addEventListener("input", () => {
        localStorage.setItem(storageKey, input.value);
    });

    input.form?.addEventListener("submit", () => {
        localStorage.removeItem(storageKey);
    });
})();
</script>)";
}

} // namespace drogular