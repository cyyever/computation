/*!
 * \file endmarkered_dpda.cpp
 */

#include "endmarkered_dpda.hpp"

namespace cyy::computation {

  void endmarkered_DPDA::convert() {
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
  /* DPDA to_DPDA() const {} */
  void endmarkered_DPDA::normalize_transitions() {
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
    // TODO add input change
  }
} // namespace cyy::computation
