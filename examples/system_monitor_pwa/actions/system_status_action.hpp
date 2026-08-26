#pragma once

#include <drogular/action_handler.hpp>

namespace system_monitor {

class SystemStatusAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override;
};

} // namespace system_monitor