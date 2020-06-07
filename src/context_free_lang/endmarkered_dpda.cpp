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
} // namespace cyy::computation
