#pragma once

#include "data/mini_store_store.hpp"
#include "features/staff/support/order_support.hpp"
#include "features/staff/pages/order_details_page.hpp"
#include "ui/mini_store_page_fragment_action.hpp"

#include <drogular/action_renderer.hpp>
#include <drogular/action_handler.hpp>

class MiniStoreAdvanceOrderAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        const auto id = context.form<int>("id").value_or(0);
        const auto returnUrl = MiniStoreOrderSupport::safeOrderReturnUrl(
            context.form<std::string>("returnUrl").value_or("")
        );
        const auto detailsUrl = MiniStoreOrderSupport::orderUrl(id, returnUrl);

        if (!context.requireService<MiniStoreStore>()->advanceOrder(id)) {
            if (context.isInteraction()) {
                return renderDetails(context, id, returnUrl, drogon::k422UnprocessableEntity);
            }
            return drogular::ActionResult::redirect(detailsUrl + "&error=transition");
        }
        if (context.isInteraction()) {
            return renderDetails(context, id, returnUrl);
        }
        return drogular::ActionResult::redirect(detailsUrl + "&success=advanced");
    }

private:
    static drogular::ActionResult renderDetails(
        drogular::ActionContext& context,
        int id,
        const std::string& returnUrl,
        drogon::HttpStatusCode status = drogon::k200OK
    ) {
        return drogular::ActionRenderer::render<
            MiniStoreStaffPageFragmentComponent<MiniStoreStaffOrderDetailsPage>
        >(
            context,
            [&](drogular::RenderContext& renderContext) {
                MiniStoreStaffOrderDetailsPage::apply(renderContext, id, returnUrl);
            },
            status
        );
    }
};
