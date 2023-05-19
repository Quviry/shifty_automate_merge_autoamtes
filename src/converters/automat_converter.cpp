#include <string>
#include <unordered_map>
#include <vector>

#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/parse/common_containers.hpp>

#include "automat_converter.hpp"

using namespace userver;

State Parse(const formats::json::Value& json, formats::parse::To<State>) {
  if (json.IsString()) {
    return State(json.As<std::string>());
  }
  return State();
}

Signal Parse(const formats::json::Value& json, formats::parse::To<Signal>) {
  if (json.IsString()) {
    return Signal(json.As<std::string>());
  }
  return Signal();
}

Automat Parse(const formats::json::Value& json, formats::parse::To<Automat>) {
  Automat automat;
  automat.initial_state = json["initial_state"].As<State>();
  automat.states = json["state"].As<std::set<State>>();
  automat.output_signals = json["output_signals"].As<std::set<Signal>>();
  automat.input_signals = json["input_signals"].As<std::set<Signal>>();
  automat.transition_function = [json](ControlPair cp) -> ControlPair {
    std::unordered_map<
        std::string, std::unordered_map<std::string,
                                  std::unordered_map<std::string, std::string>>>
        table = json["transition_function"]
                    .As<std::unordered_map<
                        std::string, std::unordered_map<
                                   std::string, std::unordered_map<std::string,
                                                              std::string>>>>();
    return {table[cp.first.id][cp.second.id]["state"],
            table[cp.first.id][cp.second.id]["signal"]};
  };
  return automat;
}

formats::json::Value Serialize(const State& state,
                               formats::serialize::To<formats::json::Value>) {
  formats::json::ValueBuilder builder;
  builder = state.id;
  return builder.ExtractValue();
}

formats::json::Value Serialize(const Signal& signal,
                               formats::serialize::To<formats::json::Value>) {
  formats::json::ValueBuilder builder;
  builder = signal.id;
  return builder.ExtractValue();
}

formats::json::Value Serialize(const Automat& data,
                               formats::serialize::To<formats::json::Value>) {
  formats::json::ValueBuilder builder;
  builder["initial_state"] = data.initial_state.id;
  for (auto const& state : data.states) {
    builder["states"].PushBack(state);
  }
  for (auto const& signal : data.input_signals) {
    builder["states"].PushBack(signal);
  }
  for (auto const& signal : data.output_signals) {
    builder["states"].PushBack(signal);
  }
  for (auto const& state : data.states) {
    for (auto const& signal : data.input_signals) {
      auto [o_state, o_signal] = data.transition_function({state, signal});
      builder["transition_function"][state.id][signal.id]["state"] = o_state.id;
      builder["transition_function"][state.id][signal.id]["signal"] =
          o_signal.id;
    }
  }
  return builder.ExtractValue();
}