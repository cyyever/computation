/*!
 * \file canonical_lr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "canonical_lr_grammar.hpp"

#include "alphabet/endmarked_alphabet.hpp"
#include "alphabet/number_set_alphabet.hpp"
#include "endmarked_dpda.hpp"

namespace cyy::computation {

  DPDA canonical_LR_grammar::to_DPDA() const {
    auto dpda_alphabet = std::make_shared<endmarked_alphabet>(alphabet);
    finite_automaton dpda_finite_automaton{{0}, dpda_alphabet, 0, {}};

    DK_1_DFA dk_1_dfa(*this);
    auto const &dfa = dk_1_dfa.get_dfa();
    auto state_symbol_set = dfa.get_state_symbol_set();
    auto dk_state_set_alphabet =
        std::make_shared<number_set_alphabet>(state_symbol_set, "dk_state_set");

    DPDA::transition_function_type transition_function;
    auto lookahead_state = dpda_finite_automaton.add_new_state();
    transition_function[dpda_finite_automaton.get_start_state()][{}] = {
        lookahead_state, dfa.get_start_state()};

    auto dfa_goto_table = dk_1_dfa.get_goto_table();
    auto reject_dfa_state = dk_1_dfa.get_reject_state();

    std::unordered_map<symbol_type, state_type> reduce_states;
    for (auto const input_symbol : dpda_alphabet->get_view()) {
      auto reduce_state = dpda_finite_automaton.add_new_state();
      reduce_states[input_symbol] = reduce_state;
      transition_function[lookahead_state][{input_symbol}] = {reduce_state};
    }

    auto accept_state = dpda_finite_automaton.add_new_state();
    for (auto const &[input_symbol, reduce_state] : reduce_states) {
      for (auto const dk_state : state_symbol_set) {
        auto const &from_lr_1_item_set = dk_1_dfa.get_LR_1_item_set(dk_state);
        auto completed_item_ptr =
            from_lr_1_item_set.get_completed_item(input_symbol);
        if (!completed_item_ptr ||
            !completed_item_ptr->contain_lookahead_symbol(input_symbol)) {

          state_type to_dfa_state = reject_dfa_state;
          auto it = dfa_goto_table.find({dk_state, input_symbol});
          if (it != dfa_goto_table.end()) {
            to_dfa_state = it->second;
          }

          transition_function.check_stack_and_action(
              reduce_state, {{}, dk_state}, {lookahead_state, to_dfa_state},
              dpda_finite_automaton);
          continue;
        }

        auto const &head = completed_item_ptr->get_head();
        auto const &body = completed_item_ptr->get_body();
        // pop body states from stack
        auto from_state = reduce_state;
        for (size_t i = 0; i < body.size(); i++) {
          auto to_state = dpda_finite_automaton.add_new_state();

          if (i == 0) {
            transition_function[from_state][{{}, dk_state}] = {to_state};
          } else {
            transition_function.pop_stack_and_action(from_state, {to_state},
                                                     *dk_state_set_alphabet);
          }
          from_state = to_state;
        }

        for (auto const prev_dk_state : state_symbol_set) {
          if (body.empty() && prev_dk_state != dk_state) {
            continue;
          }
          auto destination_state = reduce_state;
          if (prev_dk_state == dfa.get_start_state() &&
              head == get_start_symbol() &&
              input_symbol == ALPHABET::endmarker) {
            destination_state = accept_state;
          }
          transition_function.check_stack_and_action(
              from_state, {{}, prev_dk_state},
              {destination_state, dfa_goto_table[{prev_dk_state, head}]},
              dpda_finite_automaton);
        }
      }
    }

    dpda_finite_automaton.replace_final_states(accept_state);
    for (auto const input_symbol : dpda_alphabet->get_view()) {
      transition_function[accept_state][{input_symbol}] = {
          reduce_states[input_symbol]};
    }
    return endmarked_DPDA(dpda_finite_automaton, dk_state_set_alphabet,
                          transition_function);
  }

  std::pair<canonical_LR_grammar::collection_type,
            canonical_LR_grammar::goto_table_type>
  canonical_LR_grammar::get_collection() const {
    DK_1_DFA dk_1_dfa(*this);
    return {dk_1_dfa.get_LR_1_item_set_collection(), dk_1_dfa.get_goto_table()};
  }

} // namespace cyy::computation
