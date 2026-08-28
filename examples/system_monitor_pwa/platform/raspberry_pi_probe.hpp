#pragma once

#include "system/system_reader.hpp"
#include "system/system_snapshot.hpp"

#include <memory>
#include <optional>

namespace system_monitor {

class RaspberryPiProbe {
public:
    explicit RaspberryPiProbe(std::shared_ptr<SystemReader> reader);

    [[nodiscard]] std::optional<RaspberryPiInfo> read() const;

private:
    std::shared_ptr<SystemReader> reader_;
};

} // namespace system_monitor