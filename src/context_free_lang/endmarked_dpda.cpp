/*!
 * \file endmarked_dpda.cpp
 */

#include "endmarked_dpda.hpp"
#include "lang/endmarked_alphabet.hpp"
#include "lang/range_alphabet.hpp"
#include "lang/union_alphabet.hpp"
#include <algorithm>
#include <memory>

namespace cyy::computation {

  endmarked_DPDA::endmarked_DPDA(DPDA dpda) : DPDA(std::move(dpda)) {
    if (alphabet->contain(ALPHABET::endmarker)) {
      return;
    }
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

      bool has_input_epsilon = ::ranges::any_of(
          transfers, [](auto const &p) { return !p.first.input_symbol; });
      if (!has_input_epsilon) {
        transfers[{ALPHABET::endmarker}] = {next_state};
        continue;
      }

      transition_function_type::mapped_type new_transfers;
      for (const auto &[situation, action] : transfers) {
        if (!situation.use_input()) {
          continue;
        }
        assert(situation.has_pop());
        new_transfers[{ALPHABET::endmarker, situation.get_poped_symbol()}] = {
            next_state};
      }
      transfers.merge(std::move(new_transfers));
    }

    alphabet = std::make_shared<endmarked_alphabet>(alphabet);
    for (auto s : *alphabet) {
      transition_function[new_accept_state][{s}] = {reject_state_opt.value()};
    }
    replace_final_states(new_accept_state);
    check_transition_fuction();
  }
  DPDA endmarked_DPDA::to_DPDA() const {
    auto dpda = *this;
    dpda.normalize_transitions();
    auto const &const_endmarked_dpda = dpda;

    auto stack_alphabet_of_state_set = std::make_shared<range_alphabet>(
        const_endmarked_dpda.stack_alphabet->get_max_symbol() + 1,
        const_endmarked_dpda.stack_alphabet->get_max_symbol() +
            (static_cast<size_t>(1)
             << (const_endmarked_dpda.get_states().size())),
        "stack_alphabet_of_state_set");
    auto new_stack_alphabet = std::make_shared<union_alphabet>(
        const_endmarked_dpda.stack_alphabet, stack_alphabet_of_state_set);

    auto dpda_finite_automata = const_endmarked_dpda.get_finite_automata();
    assert(std::dynamic_pointer_cast<endmarked_alphabet>(
        dpda_finite_automata.get_alphabet_ptr()));
    dpda_finite_automata.set_alphabet(
        std::dynamic_pointer_cast<endmarked_alphabet>(
            dpda_finite_automata.get_alphabet_ptr())
            ->original_alphabet());
    transition_function_type dpda_transition_function;

    // Initialization
    auto new_start_state = dpda_finite_automata.add_new_state();
    auto accept_states_in_empty_stack =
        const_endmarked_dpda.get_accept_states();
    auto old_start_state = dpda_finite_automata.get_start_state();
    dpda_finite_automata.clear_final_states();

    dpda_transition_function[new_start_state][{}] = {
        old_start_state,
        stack_alphabet_of_state_set->get_min_symbol() +
            const_endmarked_dpda.get_bitset(accept_states_in_empty_stack)
                .to_ulong()};

    dpda_finite_automata.change_start_state(new_start_state);

    std::unordered_map<state_type, state_type> pop_temporary_states;
    for (auto const &[from_state, dpda_transfers] :
         const_endmarked_dpda.transition_function) {
      for (auto const &[situation, action] : dpda_transfers) {
        // pop
        if (situation.has_pop()) {
          assert(!situation.use_input());
          assert(!action.has_push());
          if (!pop_temporary_states.contains(from_state)) {
            pop_temporary_states[from_state] =
                dpda_finite_automata.add_new_state();
          }
          auto pop_temporary_state = pop_temporary_states[from_state];
          dpda_transition_function.pop_stack_and_action(
              from_state, {pop_temporary_state}, *new_stack_alphabet);

          dpda_transition_function.pop_stack_and_action(
              pop_temporary_state, action, *new_stack_alphabet);

          continue;
        }
        if (situation.use_input()) {
          continue;
        }

        // push
        assert(!situation.use_input());
        assert(!situation.has_pop());
        for (auto old_stack_symbol : *new_stack_alphabet) {
          auto new_state = dpda_finite_automata.add_new_state();
          // check stack_symbol
          dpda_transition_function[from_state][{{}, old_stack_symbol}] = {
              new_state, old_stack_symbol};
          // push original stack symbol
          auto new_state2 = dpda_finite_automata.add_new_state();
          dpda_transition_function[new_state][{}] = {new_state2,
                                                     action.stack_symbol};

          state_set_type new_accept_states;
          if (stack_alphabet_of_state_set->contain(old_stack_symbol)) {
            for (auto const &[new_from_state, new_transfers] :
                 const_endmarked_dpda.get_transition_function()) {
              for (auto const &[new_situation, new_action] : new_transfers) {
                if (new_situation.stack_symbol != action.stack_symbol) {
                  continue;
                }

                if (const_endmarked_dpda.state_bitset_contains(
                        const_endmarked_dpda.get_bitset(
                            old_stack_symbol -
                            stack_alphabet_of_state_set->get_min_symbol()),
                        new_action.state)) {
                  new_accept_states.emplace(new_from_state);
                }
              }
            }
            new_accept_states.merge(
                state_set_type(accept_states_in_empty_stack));
          }
          dpda_transition_function[new_state2][{}] = {
              action.state,
              stack_alphabet_of_state_set->get_min_symbol() +
                  const_endmarked_dpda.get_bitset(new_accept_states)
                      .to_ulong()};
        }
      }
    }

    for (const auto &[from_state, dpda_transfers] :
         const_endmarked_dpda.transition_function) {
      for (auto const &[situation, action] : dpda_transfers) {
        if (!situation.use_input()) {
          continue;
        }
        assert(!action.has_push());
        assert(!situation.has_pop());
        for (auto stack_symbol : *new_stack_alphabet) {
          dpda_transition_function[from_state]
                                  [{situation.input_symbol, stack_symbol}] = {
                                      action.state, stack_symbol};
        }
      }
    }

    std::unordered_map<state_type, state_type> parallel_states;
    auto add_parallel_state = [&](state_type s) {
      if (!parallel_states.contains(s)) {
        auto parallel_state = dpda_finite_automata.add_new_state();
        parallel_states[s] = parallel_state;
        dpda_transition_function[parallel_state] = dpda_transition_function[s];
        dpda_finite_automata.add_final_state(parallel_state);
      }
      return parallel_states[s];
    };

    for (const auto &[from_state, dpda_transfers] :
         const_endmarked_dpda.transition_function) {
      for (auto const &[situation, action] : dpda_transfers) {
        if (!situation.use_input()) {
          continue;
        }
        assert(!action.has_push());
        assert(!situation.has_pop());
        auto parallel_state = add_parallel_state(action.state);
        for (auto stack_symbol : *new_stack_alphabet) {
          if (!stack_alphabet_of_state_set->contain(stack_symbol)) {
            continue;
          }
          if (const_endmarked_dpda.state_bitset_contains(
                  const_endmarked_dpda.get_bitset(
                      stack_symbol -
                      stack_alphabet_of_state_set->get_min_symbol()),
                  action.state)) {

            dpda_transition_function[from_state]
                                    [{situation.input_symbol, stack_symbol}] = {
                                        parallel_state, stack_symbol};
          }
        }
      }
    }
    if (const_endmarked_dpda.state_bitset_contains(
            const_endmarked_dpda.get_bitset(accept_states_in_empty_stack),
            old_start_state)) {
      auto final_old_start_state = add_parallel_state(old_start_state);

      for (auto &[situation, action] :
           dpda_transition_function[new_start_state]) {
        action.state = final_old_start_state;
      }
    }
    for (auto [_, parallel_state] : parallel_states) {
      for (auto &[situation, action] :
           dpda_transition_function[parallel_state]) {
        if (!situation.use_input() && parallel_states.contains(action.state)) {
          action.state = parallel_states[action.state];
        }
      }
    }
    return DPDA(dpda_finite_automata, new_stack_alphabet,
                dpda_transition_function);
  }

  endmarked_DPDA::state_set_type endmarked_DPDA::get_accept_states() const {
    state_set_map_type epsilon_transitions;
    std::map<state_type, std::set<std::vector<stack_symbol_type>>>
        accept_states_and_stacks;

    state_set_map_type epsilon_closures;
    // found states one step to finial states
    for (const auto &[from_state, transfers] : transition_function) {
      for (const auto &[situation, action] : transfers) {
        if (!situation.use_input() ||
            situation.input_symbol.value() != ALPHABET::endmarker) {
          continue;
        }
        if (!is_final_state(action.state)) {
          continue;
        }
        assert(!situation.has_pop());
        accept_states_and_stacks[from_state].emplace();
      }
    }

    bool flag = true;
    while (flag) {
      flag = false;
      for (const auto &[from_state, transfers] : transition_function) {
        for (const auto &[situation, action] : transfers) {
          if (situation.use_input() ||
              !accept_states_and_stacks.contains(action.state)) {
            continue;
          }
          auto const &next_stacks = accept_states_and_stacks[action.state];
          assert(!next_stacks.empty());
          if (action.has_push()) {
            for (auto const &next_stack : next_stacks) {
              if (next_stack.empty()) {
                if (accept_states_and_stacks[from_state].emplace().second) {
                  flag = true;
                }
              } else if (next_stack.back() == action.get_pushed_symbol()) {
                auto cur_stack = next_stack;
                cur_stack.pop_back();
                if (accept_states_and_stacks[from_state]
                        .emplace(std::move(cur_stack))
                        .second) {
                  flag = true;
                }
              }
            }
            continue;
          }
          assert(situation.has_pop());
          for (auto const &next_stack : next_stacks) {
            auto cur_stack = next_stack;
            cur_stack.push_back(situation.get_poped_symbol());
            if (accept_states_and_stacks[from_state]
                    .emplace(std::move(cur_stack))
                    .second) {
              flag = true;
            }
          }
        }
      }
    }
    state_set_type accept_states;
    for (const auto &[state, stacks] : accept_states_and_stacks) {
      if (stacks.contains({})) {
        accept_states.insert(state);
      }
    }
    return accept_states;
  }

  void endmarked_DPDA::normalize_transitions() {
    if (transition_normalized) {
      return;
    }
    check_transition_fuction();

    // process input
    transition_function_type new_transitions;
    for (auto &[from_state, transfers] : transition_function) {
      transition_function_type::mapped_type new_transfers;
      bool has_input_epsilon = ::ranges::any_of(
          transfers, [](auto const &p) { return !p.first.use_input(); });
      bool has_stack_epsilon = ::ranges::any_of(
          transfers, [](auto const &p) { return !p.first.has_pop(); });
      std::map<stack_symbol_type, state_type> parallel_stack_states;
      std::map<input_symbol_type, state_type> parallel_input_states;

      for (auto &[situation, action] : transfers) {
        if (!situation.use_input()) {
          new_transfers[std::move(situation)] = std::move(action);
          continue;
        }
        auto input_symbol = situation.input_symbol.value();
        if (has_input_epsilon) {
          assert(!has_stack_epsilon);
          assert(situation.has_pop());
          auto stack_symbol = situation.get_poped_symbol();
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
        if (situation.has_pop()) {
          // a a
          // a b
          // ... a z
          // read and pop
          if (!parallel_input_states.contains(input_symbol)) {
            parallel_input_states[input_symbol] = add_new_state();
          }

          auto next_state = parallel_input_states[input_symbol];
          new_transfers[input_symbol] = {next_state};
          new_transitions[next_state][{{}, situation.get_poped_symbol()}] =
              std::move(action);
          continue;
        }

        // a epsilon
        if (!action.has_push()) {
          new_transfers[std::move(situation)] = std::move(action);
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
    check_transition_fuction();

    new_transitions = {};
    // process stack
    for (auto &[from_state, transfers] : transition_function) {
      transition_function_type::mapped_type new_transfers;
      for (auto &[situation, action] : transfers) {
        if (situation.has_pop() != action.has_push()) {
          new_transfers[std::move(situation)] = std::move(action);
          continue;
        }
        if (situation.use_input() && !situation.has_pop()) {
          new_transfers[std::move(situation)] = std::move(action);
          continue;
        }
        auto next_state = add_new_state();
        if (situation.has_pop()) {
          new_transfers[std::move(situation)] = {next_state};
          new_transitions[next_state][{}] = std::move(action);
          continue;
        }
        for (auto stack_symbol : *stack_alphabet) {
          new_transfers[std::move(situation)] = {next_state, stack_symbol};
          new_transitions[next_state][{{}, stack_symbol}] = std::move(action);
        }
      }
      transfers = std::move(new_transfers);
    }
    transition_function.merge(std::move(new_transitions));
    check_transition_fuction();

#ifndef NDEBUG
    for (auto &[from_state, transfers] : transition_function) {
      for (auto &[situation, action] : transfers) {
        size_t cnt = 0;
        if (situation.use_input()) {
          cnt++;
        }
        if (situation.has_pop()) {
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

  void endmarked_DPDA::prepare_DCFG_conversion() {

    assert(final_states.size() == 1);
    auto state_of_clearing_stack = add_new_state();
    for (auto final_state : final_states) {
      transition_function.erase(final_state);
      transition_function.add_epsilon_transition(final_state,
                                                 {state_of_clearing_stack});
    }

    for (auto const used_stack_symbol : *stack_alphabet) {
      transition_function[state_of_clearing_stack][{{}, used_stack_symbol}] = {
          state_of_clearing_stack};
    }
    auto new_accept_state = add_new_state();
    transition_function[state_of_clearing_stack][{{}, ALPHABET::endmarker}] = {
        new_accept_state};
    for (auto s : *alphabet) {
      transition_function[new_accept_state][{s}] = {reject_state_opt.value()};
    }
    replace_final_states(new_accept_state);

    check_transition_fuction();

    transition_function_type new_transition;

    auto placeholder_stack_symbol = stack_alphabet->get_min_symbol();
    for (auto &[from_state, transfers] : transition_function) {
      decltype(transfers) new_transfers;
      for (auto &[situation, action] : transfers) {
        // pop or push
        if (situation.has_pop() != action.has_push()) {
          new_transfers.emplace(situation, std::move(action));
          continue;
        }

        // no pop and push
        if (!situation.has_pop()) {
          assert(!action.has_push());
          auto new_state = add_new_state();
          // push
          new_transfers.emplace(
              situation, action_type{new_state, placeholder_stack_symbol});
          // pop
          for (auto stack_symbol : *stack_alphabet) {
            new_transition[new_state][{{}, stack_symbol}] = std::move(action);
          }
          continue;
        }

        // pop and push
        assert(situation.has_pop());
        assert(action.has_push());
        auto new_state = add_new_state();
        // pop
        new_transfers.emplace(situation, action_type{new_state});
        // push
        new_transition[new_state][{}] = std::move(action);
      }
      transfers = std::move(new_transfers);
    }

    transition_function.merge(std::move(new_transition));
#ifndef NDEBUG
    for (auto &[from_state, transfers] : transition_function) {
      for (auto &[situation, action] : transfers) {
        assert(situation.has_pop() != action.has_push());
      }
    }
    check_transition_fuction();
#endif
  }
} // namespace cyy::computation
