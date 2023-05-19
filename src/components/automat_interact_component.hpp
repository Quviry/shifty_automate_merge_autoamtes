#pragma once

#include <string_view>

#include <userver/components/loggable_component_base.hpp>

#include "../models/automat.hpp"

namespace components {
class InteractComponent : public userver::components::LoggableComponentBase {
 public:
  InteractComponent(const userver::components::ComponentConfig& config,
                    const userver::components::ComponentContext& context);
  static constexpr std::string_view kName = "interact-machine";

  Signal Process(Signal in);
  std::string MakeScreen(Signal in);

 private:
  void UpdateSecondAutomat();
  void UpdateFirstAutomat();

  Automat controller;
  Automat automat1;
  Automat automat2;
};
}  // namespace components