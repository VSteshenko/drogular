#pragma once

#include <string>
#include <utility>
#include <vector>

class PortalDatasetValidationResult {
public:
    void addError(std::string error) {
        errors_.push_back(std::move(error));
    }

    bool valid() const {
        return errors_.empty();
    }

    const std::vector<std::string>& errors() const {
        return errors_;
    }

private:
    std::vector<std::string> errors_;
};