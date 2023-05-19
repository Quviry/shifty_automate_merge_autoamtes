// automat.hpp
#pragma once

#include <functional>
#include <set>
#include <string>
#include <utility>
#include <vector>

const std::string kNullStateId = "null";
const std::string kNullSignalId = "null";

class AutomatException : std::exception {
  std::string description;

 public:
  AutomatException(std::string description) : description{description} {};
  char const* what() const throw() { return description.c_str(); }
};

class Signal {
 public:
  Signal(Signal signal1, Signal signal2);
  Signal(std::string id = kNullSignalId,
         std::vector<Signal> source_signals = {})
      : id{id}, source_signals{source_signals} {};
  friend bool operator<(const Signal& lhs, const Signal& rhs);
  bool is_stable();
  bool friend operator==(const Signal&, const Signal&);
  bool friend operator!=(const Signal&, const Signal&);
  std::string id;
  std::vector<Signal> source_signals{};
};

class State {
 public:
  State(State state1, State state2);
  State(std::string id = kNullStateId, std::vector<State> source_states = {})
      : id{id}, source_states{source_states} {};
  friend bool operator<(const State& lhs, const State& rhs);
  friend bool operator!=(const State& lhs, const State& rhs);
  friend bool operator==(const State& lhs, const State& rhs);
  std::string id;
  std::vector<State> source_states{};
};

using ControlPair = std::pair<State, Signal>;

bool operator<(const ControlPair& lhs, const ControlPair& rhs);

class Automat {
 public:
  Automat(std::set<Signal> input_signals, std::set<State> states,
          State initial_state,
          std::function<ControlPair(ControlPair)> transition_function,
          std::set<Signal> output_signals)
      : input_signals{input_signals},
        states{states},
        initial_state{initial_state},
        transition_function{transition_function},
        output_signals{output_signals} {};
  Automat();
  Automat& operator*=(const Automat& rhs) = delete;
  friend Automat operator*(const Automat& lhs, const Automat& rhs);
  friend bool operator==(const Automat& lhs, const Automat& rhs);
  friend std::ostream& operator<<(std::ostream& out, const Automat& point);
  std::set<Signal> input_signals;
  std::set<State> states;
  State initial_state;
  std::function<ControlPair(ControlPair)> transition_function;
  std::set<Signal> output_signals;
  std::optional<State> current_state;
};