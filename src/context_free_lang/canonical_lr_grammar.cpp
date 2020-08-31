/*!
 * \file canonical_lr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "canonical_lr_grammar.hpp"
#include "dk_1.hpp"
#include "exception.hpp"
#include "lang/number_set_alphabet.hpp"

namespace cyy::computation {
  canonical_LR_grammar::canonical_LR_grammar(ALPHABET_ptr alphabet_,
                                             nonterminal_type start_symbol_,
                                             production_set_type productions_)

      : LR_1_grammar(alphabet_, start_symbol_, std::move(productions_)) {}

  DPDA canonical_LR_grammar::to_DPDA() const {
    finite_automata dpda_finite_automata{{0}, alphabet, {0}, {}};

    DK_1_DFA dk_1_dfa(*this);
    auto const &dfa = dk_1_dfa.get_dfa();
    std::set<symbol_type> state_symbol_set;
    for (auto const s : dfa.get_states()) {
      state_symbol_set.insert(s);
    }
    auto dk_state_set_alphabet =
        std::make_shared<number_set_alphabet>(state_symbol_set, "dk_state_set");

    DPDA::transition_function_type transition_function;
    auto looping_state = dpda_finite_automata.add_new_state();
    transition_function[dpda_finite_automata.get_start_state()][{}] = {
        looping_state, dfa.get_start_state()};

    auto accept_state = dpda_finite_automata.add_new_state();
    auto goto_table = dk_1_dfa.get_goto_table();
    for (auto const dk_state : state_symbol_set) {
      // shift
      if (!dfa.is_final_state(dk_state)) {
        for (auto const input_symbol : *alphabet) {
          for (auto from_state : {looping_state, accept_state}) {
            transition_function.check_stack_and_action(
                from_state, {input_symbol, dk_state},
                {looping_state, goto_table[{dk_state, input_symbol}]},
                dpda_finite_automata);
          }
        }
        continue;
      }
      // reduce
      auto dk_final_state = dk_state;
      auto const &item =
          *(dk_1_dfa.get_LR_1_item_set(dk_state).get_completed_items().begin());
      auto const &head = item.get_head();
      auto const &body = item.get_body();

      auto reduction_states = std::vector{accept_state, looping_state};
      if (head == get_start_symbol()) {
        transition_function.check_stack_and_action(
            looping_state, {{}, dk_final_state}, {accept_state},
            dpda_finite_automata);
        reduction_states.pop_back();
      }

      for (auto reduction_state : reduction_states) {
        if (body.empty()) {
          transition_function.check_stack_and_action(
              reduction_state, {{}, dk_final_state},
              {reduction_state, goto_table[{dk_final_state, head}]},
              dpda_finite_automata);
        } else {
          auto from_state = reduction_state;
          state_type to_state;
          // pop body states from stack
          for (size_t i = 0; i < body.size(); i++) {
            to_state = dpda_finite_automata.add_new_state();
            if (i == 0) {
              transition_function[from_state][{{}, dk_final_state}] = {
                  to_state};
            } else {
              transition_function.pop_stack_and_action(from_state, {to_state},
                                                       *dk_state_set_alphabet);
            }
            from_state = to_state;
          }

          for (auto const prev_dk_state : state_symbol_set) {
            transition_function[from_state][{{}, prev_dk_state}] = {
                reduction_state, goto_table[{prev_dk_state, head}]};
          }
        }
      }
    }
    dpda_finite_automata.replace_final_states(accept_state);
    return DPDA(dpda_finite_automata, dk_state_set_alphabet,
                transition_function);
  }

  std::pair<canonical_LR_grammar::collection_type,
            canonical_LR_grammar::goto_table_type>
  canonical_LR_grammar::get_collection() const {
    DK_1_DFA dk_1_dfa(*this);
    auto const &collection = dk_1_dfa.get_LR_1_item_set_collection();
    auto goto_table = dk_1_dfa.get_goto_table();
    return {collection, goto_table};
  }

} // namespace cyy::computation
