/*!
 * \file endmarkered_dpda.cpp
 */

#include "endmarkered_dpda.hpp"
#include "lang/range_alphabet.hpp"
#include "lang/union_alphabet.hpp"

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
  DPDA endmarkered_DPDA::to_DPDA() const {
    auto dpda = *this;

    dpda.normalize_transitions();

    auto stack_alphabet_of_state_set = std::make_shared<range_alphabet>(
        dpda.stack_alphabet->get_max_symbol() + 1,
        dpda.stack_alphabet->get_max_symbol() +
            (static_cast<size_t>(1) << (dpda.get_states().size())),
        "stack_alphabet_of_state_set");

    auto accept_states = dpda.get_accept_states();
    dpda.final_states.clear();
    state_set_type old_states = dpda.get_state_set();
    auto accept_state_set_bitset = dpda.state_set_to_bitset(accept_states);
    auto new_start_state = dpda.add_new_state();
    const auto old_transition_function = dpda.transition_function;
    dpda.transition_function[new_start_state][{}] = {
        get_start_state(), stack_alphabet_of_state_set->get_min_symbol() +
                               accept_state_set_bitset.to_ulong()};
    dpda.set_start_state(new_start_state);

    for (auto &[from_state, old_transfers] : old_transition_function) {
      transition_function_type::mapped_type new_transfers;
      for (auto [situation, action] : old_transfers) {
        if (situation.has_pop()) {
          auto new_state = dpda.add_new_state();
          for (auto new_stack_symbol : *stack_alphabet_of_state_set) {
            new_transfers[{{}, new_stack_symbol}] = {new_state};
          }
          for (auto stack_symbol : *stack_alphabet) {
            new_transfers[{{}, stack_symbol}] = {reject_state_opt.value()};
          }
          dpda.transition_function[new_state][std::move(situation)] =
              std::move(action);
          continue;
        }
        if (situation.use_input()) {
          assert(!action.has_push());
          assert(!situation.has_pop());
          if (situation.input_symbol.value() != ALPHABET::endmarker) {
            continue;
          }
          for (auto new_stack_symbol : *stack_alphabet_of_state_set) {
            auto new_state = dpda.add_new_state();
            new_transfers[{{}, new_stack_symbol}] = {new_state};
            if ((state_bitset_type(old_states.size(), {action.state}) |
                 state_bitset_type(old_states.size(), new_stack_symbol))
                    .any()) {
              dpda.add_final_states(new_state);
            }
            dpda.transition_function[new_state][{}] = {action.state,
                                                       new_stack_symbol};
          }
          for (auto stack_symbol : stack_alphabet->get_view(true)) {
            auto new_state = dpda.add_new_state();
            new_transfers[{{}, stack_symbol}] = {new_state};
            dpda.transition_function[new_state][{}] = {action.state,
                                                       stack_symbol};
          }
          continue;
        }
        // push
        assert(action.has_push());
        assert(old_transfers.size() == 1);
        auto new_stack_symbol = action.stack_symbol.value();
        auto next_state = action.state;
        for (auto old_stack_symbol : *stack_alphabet_of_state_set) {
          auto new_state = dpda.add_new_state();
          new_transfers[{{}, old_stack_symbol}] = {new_state, old_stack_symbol};
          auto new_state2 = dpda.add_new_state();
          dpda.transition_function[new_state][{}] = {new_state2,
                                                     new_stack_symbol};
          state_set_type new_accept_states;
          for (auto const &[old_from_state, old_transfers] :
               old_transition_function) {
            if (old_transfers.begin()->first.stack_symbol != new_stack_symbol) {
              continue;
            }

            if (dpda.state_biset_contains(
                    state_bitset_type(old_states.size(), old_stack_symbol),
                    old_transfers.begin()->second.state)) {
              new_accept_states.emplace(old_from_state);
              break;
            }
          }
          new_accept_states.merge(state_set_type(accept_states));
          dpda.transition_function[new_state2][{}] = {
              next_state,
              stack_alphabet_of_state_set->get_min_symbol() +
                  dpda.state_set_to_bitset(old_states, new_accept_states)
                      .to_ulong()};
        }
        for (auto stack_symbol : stack_alphabet->get_view(true)) {
          new_transfers[{{}, stack_symbol}] = {reject_state_opt.value()};
        }
      }
      dpda.transition_function[from_state] = std::move(new_transfers);
    }
    auto new_stack_alphabet = std::make_shared<union_alphabet>(
        stack_alphabet, stack_alphabet_of_state_set);
    dpda.stack_alphabet = new_stack_alphabet;
    dpda.check_transition_fuction(false, true);
    return dpda;
  }

  endmarkered_DPDA::state_set_type endmarkered_DPDA::get_accept_states() const {

    state_set_map_type epsilon_transitions;
    state_set_type accept_states;

    state_set_map_type epsilon_closures;
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
      if (contain_final_state(get_epsilon_closure(epsilon_closures, from_state,
                                                  epsilon_transitions))) {
        accept_states.insert(from_state);
      }
    }
    accept_states.merge(state_set_type(final_states));
    return accept_states;
  }

  void endmarkered_DPDA::normalize_transitions() {
    if (transition_normalized) {
      return;
    }
    check_transition_fuction(true, true);

    // process input
    transition_function_type new_transitions;
    for (auto &[from_state, transfers] : transition_function) {
      transition_function_type::mapped_type new_transfers;
      bool has_input_epsilon = std::ranges::any_of(
          transfers, [](auto const &p) { return !p.first.use_input(); });
      bool has_stack_epsilon = std::ranges::any_of(
          transfers, [](auto const &p) { return !p.first.has_pop(); });
      std::map<stack_symbol_type, state_type> parallel_stack_states;
      std::map<input_symbol_type, state_type> parallel_input_states;

      for (auto &[configuration, action] : transfers) {
        if (!configuration.use_input()) {
          new_transfers[std::move(configuration)] = std::move(action);
          continue;
        }
        auto input_symbol = configuration.input_symbol.value();
        if (has_input_epsilon) {
          assert(!has_stack_epsilon);
          assert(configuration.has_pop());
          auto stack_symbol = configuration.stack_symbol.value();
          if (!parallel_stack_states.contains(stack_symbol)) {
            parallel_stack_states[stack_symbol] = add_new_state();
          }
          // a z
          // b z
          // ... z z
          // pop and read
          auto next_state = parallel_stack_states[stack_symbol];
          new_transfers[{{}, stack_symbol}] = {next_state};
          if (action.has_push()) {
            auto next_state2 = add_new_state();
            new_transitions[next_state][{input_symbol}] = {next_state2};
            new_transitions[next_state2][{}] = std::move(action);
          } else {
            new_transitions[next_state][{input_symbol}] = std::move(action);
          }
          continue;
        }
        assert(has_stack_epsilon);
        if (configuration.has_pop()) {
          // a a
          // a b
          // ... a z
          // read and pop
          if (!parallel_input_states.contains(input_symbol)) {
            parallel_input_states[input_symbol] = add_new_state();
          }

          auto next_state = parallel_input_states[input_symbol];
          new_transfers[input_symbol] = {next_state};
          new_transitions[next_state]
                         [{{}, configuration.stack_symbol.value()}] =
                             std::move(action);
          continue;
        }

        // a epsilon
        if (!action.has_push()) {
          new_transfers[std::move(configuration)] = std::move(action);
          continue;
        }
        if (!parallel_input_states.contains(input_symbol)) {
          parallel_input_states[input_symbol] = add_new_state();
        }

        auto next_state = parallel_input_states[input_symbol];
        new_transfers[input_symbol] = {next_state};
        new_transitions[next_state][{}] = std::move(action);
      }
      transfers = std::move(new_transfers);
    }
    transition_function.merge(std::move(new_transitions));
    check_transition_fuction(true, true);

    new_transitions = {};
    // process stack
    for (auto &[from_state, transfers] : transition_function) {
      transition_function_type::mapped_type new_transfers;
      for (auto &[configuration, action] : transfers) {
        if (configuration.stack_symbol.has_value() !=
            action.stack_symbol.has_value()) {
          new_transfers[std::move(configuration)] = std::move(action);
          continue;
        }
        if (configuration.use_input() && !configuration.has_pop()) {
          new_transfers[std::move(configuration)] = std::move(action);
          continue;
        }
        auto next_state = add_new_state();
        if (configuration.has_pop()) {
          new_transfers[std::move(configuration)] = {next_state};
          new_transitions[next_state][{}] = std::move(action);
          continue;
        }
        for (auto stack_symbol : stack_alphabet->get_view(true)) {
          new_transfers[std::move(configuration)] = {next_state, stack_symbol};
          new_transitions[next_state][{{}, stack_symbol}] = std::move(action);
        }
      }
      transfers = std::move(new_transfers);
    }
    transition_function.merge(std::move(new_transitions));
    check_transition_fuction(true, true);

#ifndef NDEBUG
    for (auto &[from_state, transfers] : transition_function) {
      for (auto &[configuration, action] : transfers) {
        size_t cnt = 0;
        if (configuration.use_input()) {
          cnt++;
        }
        if (configuration.has_pop()) {
          cnt++;
        }
        if (action.has_push()) {
          cnt++;
        }
        assert(cnt == 1);
      }
    }
#endif
    transition_normalized = true;
  }

} // namespace cyy::computation
