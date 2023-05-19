#include "automat_interact_component.hpp"
#include <fmt/format.h>
#include <algorithm>
#include <any>
#include <cctype>
#include <cstdint>
#include <exception>
#include <iterator>
#include <map>
#include <queue>
#include <random>
#include <string>
#include <userver/logging/log.hpp>

namespace {

Automat Trim(Automat in) {
  std::set<State> visited_states{};
  std::queue<State> stack;
  stack.push(in.initial_state);
  while (!stack.empty()) {
    const State state = stack.front();
    stack.pop();
    for (const auto& signal : in.input_signals) {
      auto next_state = in.transition_function({state, signal}).first;
      if (!visited_states.contains(next_state)) {
        stack.push(next_state);
      }
    }
    visited_states.insert(state);
  }

  in.states = visited_states;  // Оставляем только достижимые вершины
  return in;
}

Automat GenerateAutomat(const std::string& prefix, uint32_t count = 2) {
  std::random_device os_seed;
  const uint32_t seed = os_seed();

  std::mt19937 generator(seed);
  std::uniform_int_distribution<uint32_t> random_signal(0, 1);
  std::uniform_int_distribution<uint32_t> random_state(0, count - 1);

  Automat result;
  result.input_signals = {{"a"}, {"b"}};
  result.output_signals = {{"0"}, {"1"}};
  result.states = {};
  for (uint32_t i = 0; i < count; ++i) {
    result.states.insert(State(prefix + std::to_string(i)));
  }
  result.initial_state = prefix + "0";
  std::map<ControlPair, ControlPair> mapper{};
  for (const auto state : result.states) {
    for (const auto signal : result.input_signals) {
      mapper.insert(
          {{state, signal},
           {*std::next(result.states.begin(), random_state(generator)),
            *std::next(result.output_signals.begin(),
                       random_signal(generator))}});
    }
  }
  result.transition_function = [mapper](ControlPair in) mutable -> ControlPair {
    return mapper[in];
  };
  return result;
}

std::string AutomatDiagram(const Automat& automat) {
  std::string content = "<pre class=\"mermaid\">\ngraph TD\n";
  content += "\tstyle " + automat.initial_state.id + " fill:#1c98b6\n";
  for (const State& istate : automat.states) {
    content.append("\t" + istate.id + "(( " + istate.id + " )) \n");
    for (const Signal& isignal : automat.input_signals) {
      auto [ostate, osignal] = automat.transition_function({istate, isignal});
      content.append("\t" + istate.id + " -->|" + isignal.id + "/" +
                     osignal.id + "| " + ostate.id + " \n");
    }
  }
  content.append("</pre>");
  return content;
}
}  // namespace

namespace interact_component::routes {
const std::string kEmpty{""};
const std::string kGenerate{"generate"};
const std::string kRegenerate = "regenerate";
const std::string kCompare = "compare";
}  // namespace interact_component::routes

namespace interact_component::states {
const State Idle{"Idle"};
const State FirstAutomat{"FirstAutomat"};
const State SecondAutomat{"SecondAutomat"};
const State Comparison{"Comparison"};
const State Error{"Error"};
}  // namespace interact_component::states

namespace interact_component::signals {
const Signal Idle{"Idle"};
const Signal FirstAutomat{"FirstAutomat"};
const Signal SecondAutomat{"SecondAutomat"};
const Signal Comparison{"Comparison"};
const Signal Error{"Error"};
}  // namespace interact_component::signals

namespace interact_component::screens {

const std::string kTemplate = R"(
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Bootstrap demo</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0-alpha3/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-KK94CHFLLe+nY2dmCWGMq91rCGa5gtU4mk92HdvYe+M/SXH301p5ILy+dN9+nJOZ" crossorigin="anonymous">
  </head>
  <body style="font-family: monospace">
    <div class="col-8 mx-auto my-5 justify-content-around">
        {}
        <div class="d-flex justify-content-around my-5">
          <a class="btn btn-primary m-2" href="/" role="button">Send "" signal</a>
          <a class="btn btn-primary m-2" href="/generate" role="button">Send "generate" signal</a>
          <a class="btn btn-primary m-2" href="/regenerate" role="button">Send "regenerate" signal</a>
          <a class="btn btn-primary m-2" href="/compare" role="button">Send "compare" signal</a>
          <a class="btn btn-primary m-2" href="/oops" role="button">Send "oops" signal</a>
        <div>
    </div>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0-alpha3/dist/js/bootstrap.bundle.min.js" integrity="sha384-ENjdO4Dr2bkBIFxQpeoTz1HIcje39Wm4jDKdf19U8gI4ddQ3GYNS7NTKfAdVQSZe" crossorigin="anonymous"></script>
    <script type="module">
      import mermaid from 'https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.esm.min.mjs';
      mermaid.initialize();
    </script>
  </body>
</html>)";

const std::string MakeIdleScreen() {
  return fmt::format(kTemplate, R"(
        <div class="d-flex align-items-center my-5">
            <strong>Idling...</strong>
            <div class="spinner-border ms-auto" role="status" aria-hidden="true"></div>
        </div>)");
}

std::string MakeComparitionScreen(Automat automat1, Automat automat2) {
  const std::string top_header = "<h1>Comparison of automatons</h1><br>";
  const std::string summ_header = "<h3>Step 1: Multiply</h3><br>";
  std::string summ_content = AutomatDiagram(automat1 * automat2);
  const std::string trim_header = "<h3>Step 2: Trim</h3><br>";
  std::string trim_content = AutomatDiagram(Trim(automat1 * automat2));
  const std::string cmp_header = "<h3>Step 3: Compare</h3><br>";
  std::string cmp_content =
      std::string("<h4>Automatons are ") +
      ((automat1 == automat2) ? "Equal!</h4>" : "Unequal!</h4>");

  return fmt::format(kTemplate, top_header + summ_header + summ_content +
                                    trim_header + trim_content + cmp_header +
                                    cmp_content);
}

std::string MakeFirstAutomatScreen(Automat automat) {
  const std::string header = "<h1>Automat 1</h1><br>";
  std::string content = AutomatDiagram(automat);
  return fmt::format(kTemplate, header + content);
}

std::string MakeSecondAutomatScreen(Automat automat) {
  const std::string header = "<h1>Automat 2</h1><br>";
  std::string content = AutomatDiagram(automat);
  return fmt::format(kTemplate, header + content);
}

std::string MakeErrorScreen() {
  return fmt::format(kTemplate, "<h1>ERROR</h1><br><h3>please go back</h3>");
}

}  // namespace interact_component::screens

/*
              +      + generate      + regenerate    + compare     + <other>
Idle          | Idle | FirstAutomat  | Error         | Error       | Error
FirstAutomat  | Idle | SecondAutomat | FirstAutomat  | Error       | Error
SecondAutomat | Idle | Error         | SecondAutomat | Comparison  | Error
Comparison    | Idle | Error         | Error         | Comparison | Error
Error         | Idle | Error         | Error         | Error       | Error

*/

namespace components {
using namespace interact_component;
InteractComponent::InteractComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::LoggableComponentBase(config, context) {
  Automat self_controller{};
  self_controller.initial_state = states::Idle;
  self_controller.current_state = self_controller.initial_state;
  self_controller.input_signals = {{routes::kCompare},
                                   {routes::kEmpty},
                                   {routes::kGenerate},
                                   {routes::kRegenerate}};
  self_controller.states = {states::Idle, states::FirstAutomat,
                            states::SecondAutomat, states::Comparison,
                            states::Error};
  self_controller.output_signals = {signals::Idle, signals::FirstAutomat,
                                    signals::SecondAutomat, signals::Comparison,
                                    signals::Error};
  self_controller.transition_function = [&](ControlPair in) -> ControlPair {
    std::map<ControlPair, ControlPair> mapping = {
        {{states::Idle, {routes::kEmpty}}, {states::Idle, signals::Idle}},
        {{states::Idle, {routes::kGenerate}},
         {states::FirstAutomat, signals::FirstAutomat}},

        {{states::FirstAutomat, {routes::kEmpty}},
         {states::Idle, signals::Idle}},
        {{states::FirstAutomat, {routes::kRegenerate}},
         {states::FirstAutomat, signals::FirstAutomat}},
        {{states::FirstAutomat, {routes::kGenerate}},
         {states::SecondAutomat, signals::SecondAutomat}},

        {{states::SecondAutomat, {routes::kEmpty}},
         {states::Idle, signals::Idle}},
        {{states::SecondAutomat, {routes::kRegenerate}},
         {states::SecondAutomat, signals::SecondAutomat}},
        {{states::SecondAutomat, {routes::kCompare}},
         {states::Comparison, signals::Comparison}},

        {{states::Comparison, {routes::kEmpty}}, {states::Idle, signals::Idle}},

        {{states::Error, {routes::kEmpty}}, {states::Idle, signals::Idle}},
    };
    if (mapping.contains(in)) {
      return mapping[in];
    }
    return {states::Error, signals::Error};
  };
  controller = std::move(self_controller);
}

Signal InteractComponent::Process(Signal in) {
  auto [state, signal] =
      controller.transition_function({*controller.current_state, in});
  controller.current_state = state;
  return signal;
}

std::string InteractComponent::MakeScreen(Signal in) {
  if (in == signals::Idle) {
    return screens::MakeIdleScreen();
  }
  if (in == signals::Comparison) {
    return screens::MakeComparitionScreen(automat1, automat2);
  }
  if (in == signals::FirstAutomat) {
    UpdateFirstAutomat();
    return screens::MakeFirstAutomatScreen(automat1);
  }
  if (in == signals::SecondAutomat) {
    UpdateSecondAutomat();
    return screens::MakeSecondAutomatScreen(automat2);
  }
  return screens::MakeErrorScreen();
}

void InteractComponent::UpdateSecondAutomat() {
  automat2 = GenerateAutomat("S", 2);
}
void InteractComponent::UpdateFirstAutomat() {
  automat1 = GenerateAutomat("q", 3);
}
}  // namespace components