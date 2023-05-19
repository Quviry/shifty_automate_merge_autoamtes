#include "signal_handler.hpp"
#include <cstdlib>
#include <userver/logging/log.hpp>

namespace handlers {
SignalHandler::SignalHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerBase(config, context),
      controller_(context.FindComponent<components::InteractComponent>(
          "interact-machine")) {}

std::string SignalHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  std::string signal = request.GetUrl().substr(1); 
  if(signal == "favicon.ico")
    return "404";
  LOG_DEBUG() << "Process signal " << signal;
  Signal response_signal = controller_.Process(signal);
  return controller_.MakeScreen(response_signal);
}
}  // namespace handlers