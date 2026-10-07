#include "localization_check.hpp"

#include <drogular/file_translation_provider.hpp>

#include <exception>
#include <ostream>

namespace drogular::localization_check {
namespace {

Options invalid(std::string message) {
    Options options;
    options.error = std::move(message);
    return options;
}

} // namespace

Options parseArguments(const std::vector<std::string_view>& arguments) {
    if (arguments.empty()) {
        return invalid("Missing localization directory.");
    }

    if (arguments.size() == 1 && (arguments[0] == "--help" || arguments[0] == "-h")) {
        Options options;
        options.help = true;
        return options;
    }

    Options options;
    bool directorySpecified = false;
    bool referenceSpecified = false;

    for (std::size_t index = 0; index < arguments.size(); ++index) {
        const std::string_view argument = arguments[index];

        if (argument == "--reference" || argument == "-r") {
            if (referenceSpecified) {
                return invalid("Reference locale was specified more than once.");
            }
            if (++index >= arguments.size() || arguments[index].empty()) {
                return invalid("Missing value for --reference.");
            }
            options.referenceLocale = std::string(arguments[index]);
            referenceSpecified = true;
            continue;
        }

        constexpr std::string_view prefix = "--reference=";
        if (argument.starts_with(prefix)) {
            if (referenceSpecified) {
                return invalid("Reference locale was specified more than once.");
            }
            const auto locale = argument.substr(prefix.size());
            if (locale.empty()) {
                return invalid("Reference locale must not be empty.");
            }
            options.referenceLocale = std::string(locale);
            referenceSpecified = true;
            continue;
        }

        if (argument.starts_with("-")) {
            return invalid("Unknown option: " + std::string(argument));
        }

        if (directorySpecified) {
            return invalid("Localization directory was specified more than once.");
        }
        options.directory = std::filesystem::path(argument);
        directorySpecified = true;
    }

    if (!directorySpecified || options.directory.empty()) {
        return invalid("Missing localization directory.");
    }

    return options;
}

void printUsage(std::ostream& output) {
    output
        << "Usage: drogular-l10n-check <directory> [--reference <locale>]\n"
        << "\n"
        << "Validates locale JSON files against a reference locale.\n"
        << "The reference locale defaults to 'en'.\n";
}

void printResult(const TranslationValidationResult& result, std::ostream& output) {
    if (result.valid()) {
        output << "Localization catalog is valid (reference: "
               << result.referenceLocale << ").\n";
        return;
    }

    output << "Localization catalog validation failed (reference: "
           << result.referenceLocale << ").\n";

    for (const auto& [locale, validation] : result.locales) {
        if (validation.valid()) {
            continue;
        }

        output << locale << ":\n";
        for (const auto& key : validation.missingKeys) {
            output << "  missing: " << key << '\n';
        }
        for (const auto& key : validation.extraKeys) {
            output << "  extra:   " << key << '\n';
        }
    }
}

int run(const Options& options, std::ostream& output, std::ostream& error) {
    if (options.help) {
        printUsage(output);
        return 0;
    }

    if (!options.valid()) {
        error << "error: " << options.error << "\n\n";
        printUsage(error);
        return 2;
    }

    try {
        const FileTranslationProvider provider(options.directory, options.referenceLocale);
        const auto result = TranslationCatalogValidator::validate(provider);
        printResult(result, output);
        return result.valid() ? 0 : 1;
    } catch (const std::exception& exception) {
        error << "error: " << exception.what() << '\n';
        return 2;
    }
}

} // namespace drogular::localization_check