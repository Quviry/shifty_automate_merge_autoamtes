// automat.cpp
#include "automat.hpp"
#include <iomanip>
#include <iostream>
#include <queue>
#include <vector>

Signal::Signal(Signal signal1, Signal signal2) {
  id = signal1.id + '_' + signal2.id;
  source_signals.emplace_back(signal1);
  source_signals.emplace_back(signal2);
}

State::State(State state1, State state2) {
  id = state1.id + '_' + state2.id;
  source_states.emplace_back(state1);
  source_states.emplace_back(state2);
}

bool operator<(const ControlPair& lhs, const ControlPair& rhs) {
  return (lhs.first.id + lhs.second.id) < (rhs.first.id + rhs.second.id);
}

Automat::Automat() {
  input_signals = {};
  states = {};
  initial_state = State();
  std::function<ControlPair(ControlPair)> transition_function =
      [](ControlPair) { return ControlPair(State(), Signal()); };
  std::set<Signal> output_signals = {};
}

Automat operator*(const Automat& lhs, const Automat& rhs) {
  Automat result{};
  if (lhs.input_signals != rhs.input_signals) {
    throw AutomatException("Input signals are unequal");
  }

  if (lhs.output_signals != rhs.output_signals) {
    throw AutomatException("Output signals are unequal");
  }

  result.input_signals = lhs.input_signals;
  result.output_signals = lhs.output_signals;

  for (auto state1 : lhs.states) {
    for (auto state2 : rhs.states) {
      result.states.insert(State(state1, state2));
    }
  }

  result.initial_state = State(lhs.initial_state, rhs.initial_state);

  result.transition_function = [&](ControlPair control) {
    const auto& first = lhs.transition_function(
        {control.first.source_states[0], control.second});
    const auto& second = rhs.transition_function(
        {control.first.source_states[1], control.second});
    return ControlPair(State(first.first, second.first),
                       Signal(first.second, second.second));
  };

  return result;
}

bool operator==(const Signal& lhs, const Signal& rhs) {
  if (lhs.id != rhs.id) return false;
  if (lhs.source_signals.size() != rhs.source_signals.size()) return false;
  for (size_t i = 0; i < lhs.source_signals.size(); ++i) {
    if (!(lhs.source_signals[i] == rhs.source_signals[i])) return false;
  }
  return true;
}

bool operator==(const State& lhs, const State& rhs) {
  if (lhs.id != rhs.id) return false;
  if (lhs.source_states.size() != rhs.source_states.size()) return false;
  for (size_t i = 0; i < lhs.source_states.size(); ++i) {
    if (!(lhs.source_states[i] == rhs.source_states[i])) return false;
  }
  return true;
}

bool operator!=(const Signal& lhs, const Signal& rhs) { return !(lhs == rhs); }

bool operator!=(const State& lhs, const State& rhs) { return !(lhs == rhs); }

bool Signal::is_stable() {
  if (source_signals.size() < 2) return true;
  return std::equal(source_signals.begin() + 1, source_signals.end(),
                    source_signals.begin());
}

bool operator<(const Signal& lhs, const Signal& rhs) { return lhs.id < rhs.id; }

bool operator<(const State& lhs, const State& rhs) { return lhs.id < rhs.id; }

std::ostream& operator<<(std::ostream& out, const Automat& Automat) {
  const uint32_t kFieldWidth = 10;
  out << "Automat table:\n";
  std::vector<Signal> signals{};
  signals.reserve(Automat.input_signals.size());
  for (uid_t i = 0; i < kFieldWidth; ++i) out << " ";
  for (const auto& signal : Automat.input_signals) {
    signals.emplace_back(signal);
    out << std::setw(kFieldWidth) << signal.id << std::setw(kFieldWidth)
        << signal.id;
  }
  out << std::endl;
  for (const auto& state : Automat.states) {
    if (state.id != Automat.initial_state.id) {
      out << std::setw(kFieldWidth - 1) << state.id << "|";
    } else {
      out << "\x1B[36m" << std::setw(kFieldWidth - 1) << state.id << "|\033[0m";
    }
    for (const auto& signal : signals) {
      auto [f_state, f_signal] = Automat.transition_function({state, signal});
      out << std::setw(kFieldWidth) << f_state.id << std::setw(kFieldWidth)
          << f_signal.id;
    }
    out << std::endl;
  }
  return out;
}

bool operator==(const Automat& lhs, const Automat& rhs) {
  Automat merged = lhs * rhs;  // Получаем произведение автоматов

#ifdef DEBUG
  std::cout << merged << std::endl;
#endif  // DEBUG

  std::set<State> visited_states{};
  std::queue<State> stack;
  stack.push(merged.initial_state);
  while (!stack.empty()) {
    const State state = stack.front();
    stack.pop();
    for (const auto& signal : merged.input_signals) {
      auto next_state = merged.transition_function({state, signal}).first;
      if (!visited_states.contains(next_state)) {
        stack.push(next_state);
      }
    }
    visited_states.insert(state);
  }

  merged.states = visited_states;  // Оставляем только достижимые вершины

#ifdef DEBUG
  std::cout << merged << std::endl;
#endif  // DEBUG

  for (const auto& state : merged.states) {
    for (const auto& signal : merged.input_signals) {
      // Если сигналы не совпадают, то автоматы неэквивалентны
      if (!merged.transition_function({state, signal}).second.is_stable())
        return false;
    }
  }
  return true;
}
