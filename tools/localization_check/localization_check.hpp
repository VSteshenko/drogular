#pragma once

#include <drogular/translation_catalog_validator.hpp>

#include <filesystem>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace drogular::localization_check {

struct Options {
    std::filesystem::path directory;
    std::string referenceLocale{"en"};
    bool help{false};
    std::string error;

    bool valid() const noexcept { return error.empty(); }
};

Options parseArguments(const std::vector<std::string_view>& arguments);

void printUsage(std::ostream& output);
void printResult(const TranslationValidationResult& result, std::ostream& output);

int run(const Options& options, std::ostream& output, std::ostream& error);

} // namespace drogular::localization_check