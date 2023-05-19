
#pragma once

#include <string>
#include <string_view>

#include "../components/automat_interact_component.hpp"

#include <userver/components/component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

namespace handlers {
class SignalHandler final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-signal";

  using HttpHandlerBase::HttpHandlerBase;

  SignalHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext&) const override;
    private:
    components::InteractComponent& controller_; 
    
};

}  // namespace handlers
