/*!
 * \file endmarkered_dpda.cpp
 */

#include <unordered_map>

#include "endmarkered_dpda.hpp"
#include "lang/range_alphabet.hpp"

namespace cyy::computation {

  void endmarkered_DPDA::to_endmarkered_DPDA() {
    normalize();

    auto new_accept_state = add_new_state();

    for (auto &[from_state, transfers] : transition_function) {

      if (transfers.contains({})) {
        continue;
      }

      state_type next_state{};
      if (is_final_state(from_state)) {
        next_state = new_accept_state;
      } else {
        next_state = reject_state_opt.value();
      }

      bool has_input_epsilon = std::ranges::any_of(
          transfers, [](auto const &p) { return !p.first.input_symbol; });
      if (!has_input_epsilon) {
        transfers[{ALPHABET::endmarker}] = {next_state};
        continue;
      }

      transition_function_type::mapped_type new_transfers;
      for (const auto &[situation, action] : transfers) {
        if (!situation.input_symbol) {
          continue;
        }
        assert(situation.stack_symbol.has_value());
        new_transfers[{ALPHABET::endmarker, situation.stack_symbol.value()}] = {
            next_state};
      }
      transfers.merge(std::move(new_transfers));
    }

    for (auto s : alphabet->get_view(true)) {
      transition_function[new_accept_state][{s}] = {reject_state_opt.value()};
    }

    final_states = {new_accept_state};
    check_transition_fuction(true, true);
  }
  void endmarkered_DPDA::to_DPDA() {
    normalize_transitions();

    auto stack_alphabet_of_state_set = std::make_shared<range_alphabet>(
        stack_alphabet->get_max_symbol() + 1,
        stack_alphabet->get_max_symbol() +
            (static_cast<size_t>(1) << (states.size())),
        "stack_alphabet_of_state_set");

    auto accept_states = get_accept_states();
    final_states.clear();
    auto accept_state_set_bitset = state_set_to_bitset(accept_states);
    auto new_start_state = add_new_state();
    transition_function[new_start_state][{}] = {
        start_state, stack_alphabet_of_state_set->get_min_symbol() +
                         accept_state_set_bitset.to_ulong()};
    start_state = new_start_state;

    transition_function_type new_transitions;
    for (auto &[from_state, transfers] : transition_function) {
      transition_function_type::mapped_type new_transfers;
      for (auto &[situation, action] : transfers) {
        if (situation.has_pop()) {
          auto new_state = add_new_state();
          for (auto new_stack_symbol : *stack_alphabet_of_state_set) {
            new_transfers[{{}, new_stack_symbol}] = {new_state};
          }
          new_transitions[new_state][std::move(situation)] = std::move(action);
          continue;
        }
        if (situation.use_input()) {
          if (situation.input_symbol.value() != ALPHABET::endmarker) {
            continue;
          }
          for (auto new_stack_symbol : *stack_alphabet_of_state_set) {
            if ((boost::dynamic_bitset<>(states.size(), {action.state}) |
                 boost::dynamic_bitset<>(states.size(), new_stack_symbol))
                    .any()) {
              auto new_state = add_new_state();
              add_final_states(new_state);
              new_transfers[{{}, new_stack_symbol}] = {new_state};
              assert(!action.has_push());
              new_transitions[new_state][{}] = {action.state, new_stack_symbol};
            } else {
              new_transfers[{{}, new_stack_symbol}] = action;
            }
          }
          continue;
        }
      }
      transfers = std::move(new_transfers);
    }
    transition_function.merge(std::move(new_transitions));
    return;
  }

  endmarkered_DPDA::state_set_type endmarkered_DPDA::get_accept_states() const {
    epsilon_closures.clear();

    state_set_map_type epsilon_transitions;
    state_set_type accept_states;

    for (const auto &[from_state, transfers] : transition_function) {
      for (const auto &[situation, action] : transfers) {
        if (situation.input_symbol.has_value() ||
            situation.stack_symbol.has_value()) {
          continue;
        }
        epsilon_transitions[from_state].insert(action.state);
      }
    }
    for (auto const &[from_state, _] : epsilon_transitions) {
      if (contain_final_state(
              get_epsilon_closure(from_state, epsilon_transitions))) {
        accept_states.insert(from_state);
      }
    }
    accept_states.merge(state_set_type(final_states));
    epsilon_closures.clear();
    return accept_states;
  }

  void endmarkered_DPDA::normalize_transitions() {
    if (transition_normalized) {
      return;
    }

    auto placeholder_stack_symbol = stack_alphabet->get_min_symbol();
    transition_function_type new_transitions;
    for (auto &[from_state, transfers] : transition_function) {
      transition_function_type::mapped_type new_transfers;
      for (auto &[configuration, action] : transfers) {
        if (configuration.stack_symbol.has_value() !=
            action.stack_symbol.has_value()) {
          new_transfers[std::move(configuration)] = std::move(action);
          continue;
        }
        auto next_state = add_new_state();
        if (configuration.stack_symbol.has_value()) {
          new_transfers[std::move(configuration)] = {next_state};
          new_transitions[next_state][{}] = {std::move(action)};
          continue;
        }
        new_transfers[std::move(configuration)] = {next_state,
                                                   placeholder_stack_symbol};
        new_transitions[next_state][{{}, placeholder_stack_symbol}] = {
            std::move(action)};
      }
      transfers = std::move(new_transfers);
    }
    transition_function.merge(std::move(new_transitions));

    new_transitions = {};
    for (auto &[from_state, transfers] : transition_function) {
      transition_function_type::mapped_type new_transfers;
      for (auto &[configuration, action] : transfers) {
        if (!configuration.input_symbol.has_value()) {
          new_transfers[std::move(configuration)] = std::move(action);
          continue;
        }
        auto input_symbol = configuration.input_symbol.value();
        auto next_state = add_new_state();
        if (configuration.stack_symbol.has_value()) {
          new_transfers[{{}, configuration.stack_symbol.value()}] = {
              next_state};
          new_transitions[next_state][{input_symbol}] = {std::move(action)};
          continue;
        }
        new_transfers[std::move(configuration)] = {next_state};
        new_transitions[next_state][{}] = {std::move(action)};
      }
      transfers = std::move(new_transfers);
    }
    transition_function.merge(std::move(new_transitions));
    transition_normalized = true;
    check_transition_fuction(true, true);
  }
  boost::dynamic_bitset<>
  endmarkered_DPDA::state_set_to_bitset(const state_set_type &state_set) {
    boost::dynamic_bitset<> bitset(states.size());
    auto it = states.begin();
    auto it2 = state_set.begin();
    while (it != states.end() && it2 != state_set.end()) {
      if (*it == *it2) {
        bitset.set(std::distance(states.begin(), it));
        it++;
        it2++;
        continue;
      }
      if (*it < *it2) {
        it++;
        continue;
      }
      it2++;
    }
    return bitset;
  }
} // namespace cyy::computation
