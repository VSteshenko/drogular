#include "localization_check.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;
using namespace drogular::localization_check;

namespace {

class TempDirectory {
public:
    TempDirectory() {
        path_ = fs::temp_directory_path() / "drogular_l10n_check_tests";
        fs::remove_all(path_);
        fs::create_directories(path_);
    }
    ~TempDirectory() { fs::remove_all(path_); }
    const fs::path& path() const { return path_; }
    void write(std::string_view locale, std::string_view json) const {
        std::ofstream(path_ / (std::string(locale) + ".json")) << json;
    }
private:
    fs::path path_;
};

} // namespace

TEST(LocalizationCheckTests, ParsesDirectoryAndReferenceLocale) {
    const auto options = parseArguments({"translations", "--reference", "de"});
    EXPECT_TRUE(options.valid());
    EXPECT_EQ(options.directory, fs::path("translations"));
    EXPECT_EQ(options.referenceLocale, "de");
}

TEST(LocalizationCheckTests, DefaultsReferenceLocaleToEnglish) {
    const auto options = parseArguments({"translations"});
    EXPECT_TRUE(options.valid());
    EXPECT_EQ(options.referenceLocale, "en");
}

TEST(LocalizationCheckTests, RejectsMissingDirectory) {
    const auto options = parseArguments({});
    EXPECT_FALSE(options.valid());
}

TEST(LocalizationCheckTests, ReturnsZeroForCompleteCatalog) {
    TempDirectory directory;
    directory.write("en", R"({"title":"Title"})");
    directory.write("de", R"({"title":"Titel"})");
    Options options{directory.path(), "en"};
    std::ostringstream output, error;
    EXPECT_EQ(run(options, output, error), 0);
    EXPECT_NE(output.str().find("catalog is valid"), std::string::npos);
    EXPECT_TRUE(error.str().empty());
}

TEST(LocalizationCheckTests, ReturnsOneAndReportsValidationIssues) {
    TempDirectory directory;
    directory.write("en", R"({"title":"Title","search":"Search"})");
    directory.write("de", R"({"title":"Titel","legacy":"Alt"})");
    Options options{directory.path(), "en"};
    std::ostringstream output, error;
    EXPECT_EQ(run(options, output, error), 1);
    EXPECT_NE(output.str().find("missing: search"), std::string::npos);
    EXPECT_NE(output.str().find("extra:   legacy"), std::string::npos);
    EXPECT_TRUE(error.str().empty());
}

TEST(LocalizationCheckTests, ReturnsTwoForLoadErrors) {
    Options options{fs::path("does-not-exist"), "en"};
    std::ostringstream output, error;
    EXPECT_EQ(run(options, output, error), 2);
    EXPECT_FALSE(error.str().empty());
}