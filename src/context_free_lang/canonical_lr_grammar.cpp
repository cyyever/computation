/*!
 * \file canonical_lr_grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */
#include "canonical_lr_grammar.hpp"

#include <memory>

#include "dk_1.hpp"
#include "endmarked_dpda.hpp"
#include "exception.hpp"
#include "lang/endmarked_alphabet.hpp"
#include "lang/number_set_alphabet.hpp"

namespace cyy::computation {
  canonical_LR_grammar::canonical_LR_grammar(ALPHABET_ptr alphabet_,
                                             nonterminal_type start_symbol_,
                                             production_set_type productions_)

      : LR_1_grammar(alphabet_, start_symbol_, std::move(productions_)),dk_1_dfa_opt(DK_1_DFA(*this)) {}
  DPDA canonical_LR_grammar::to_DPDA() const {
    auto dpda_alphabet = std::make_shared<endmarked_alphabet>(alphabet);
    finite_automata dpda_finite_automata{{0}, dpda_alphabet, 0, {}};

    auto const &dfa = dk_1_dfa_opt->get_dfa();
    symbol_set_type state_symbol_set;
    for (auto const s : dfa.get_states()) {
      assert(s <= std::numeric_limits<symbol_type>::max());
      state_symbol_set.insert(static_cast<symbol_type>(s));
    }
    auto dk_state_set_alphabet =
        std::make_shared<number_set_alphabet>(state_symbol_set, "dk_state_set");

    DPDA::transition_function_type transition_function;
    auto looping_state = dpda_finite_automata.add_new_state();
    transition_function[dpda_finite_automata.get_start_state()][{}] = {
        looping_state, dfa.get_start_state()};

    auto goto_table = dk_1_dfa_opt->get_goto_table();
    std::unordered_map<symbol_type, state_type> lookahead_states;

    for (auto const input_symbol : *dpda_alphabet) {
      lookahead_states[input_symbol] = dpda_finite_automata.add_new_state();
    }

    for (auto const dk_state : state_symbol_set) {
      auto const &lr_1_item_set = dk_1_dfa_opt->get_LR_1_item_set(dk_state);
      for (auto const input_symbol : *dpda_alphabet) {
        auto completed_item_ptr =
            lr_1_item_set.get_completed_item(input_symbol);
        DPDA::action_type action;
        if (!completed_item_ptr) {
          // shift
          action = {looping_state, goto_table[{dk_state, input_symbol}]};
        } else {
          // move to lookahead state
          auto lookahead_state = lookahead_states[input_symbol];
          action = {lookahead_state};
        }
        transition_function.check_stack_and_action(
            looping_state, {input_symbol, dk_state}, action,
            dpda_finite_automata);
      }
    }

    for (auto &[lookahead_symbol, from_state] : lookahead_states) {
      for (auto const dk_state : state_symbol_set) {
        auto const &lr_1_item_set = dk_1_dfa_opt->get_LR_1_item_set(dk_state);
        auto completed_item_ptr =
            lr_1_item_set.get_completed_item(lookahead_symbol);
        if (!completed_item_ptr) {
          // shift
          DPDA::action_type action = {looping_state,
                                      goto_table[{dk_state, lookahead_symbol}]};
          transition_function.check_stack_and_action(
              from_state, {{}, dk_state}, action, dpda_finite_automata);
          continue;
        }
        auto const &head = completed_item_ptr->get_head();
        auto const &body = completed_item_ptr->get_body();
        if (body.empty()) {
          transition_function.check_stack_and_action(
              from_state, {{}, dk_state},
              {from_state, goto_table[{dk_state, head}]}, dpda_finite_automata);
        } else {
          // pop body states from stack
          auto destination_state = from_state;
          for (size_t i = 0; i < body.size(); i++) {
            auto to_state = dpda_finite_automata.add_new_state();

            if (i == 0) {
              transition_function[from_state][{{}, dk_state}] = {to_state};
            } else {
              transition_function.pop_stack_and_action(from_state, {to_state},
                                                       *dk_state_set_alphabet);
            }
            from_state = to_state;
          }

          for (auto const prev_dk_state : state_symbol_set) {
            transition_function.check_stack_and_action(
                from_state, {{}, prev_dk_state},
                {destination_state, goto_table[{prev_dk_state, head}]},
                dpda_finite_automata);
          }
        }
      }
    }

    /* dpda_finite_automata.replace_final_states(accept_state); */
    return endmarked_DPDA(dpda_finite_automata, dk_state_set_alphabet,
                          transition_function);
  }

  std::pair<canonical_LR_grammar::collection_type,
            canonical_LR_grammar::goto_table_type>
  canonical_LR_grammar::get_collection() const {
    auto const &collection = dk_1_dfa_opt->get_LR_1_item_set_collection();
    auto goto_table = dk_1_dfa_opt->get_goto_table();
    return {collection, goto_table};
  }

} // namespace cyy::computation
