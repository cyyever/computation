#include <iostream>
#include <ranges>

#include "dcfg.hpp"
#include "lang/number_set_alphabet.hpp"
namespace cyy::computation {
  DCFG::DCFG(std::shared_ptr<ALPHABET> alphabet_,
             nonterminal_type start_symbol_, production_set_type productions_)

      : LR_0_grammar(alphabet_, start_symbol_, std::move(productions_)),
        dk_dfa_ptr(std::make_shared<DK_DFA>(*this)) {
    if (!DK_test()) {
      throw exception::no_DCFG("DK test failed");
    }
  }

  bool DCFG::DK_test() const {
    for (auto final_state : dk_dfa_ptr->get_dfa().get_final_states()) {
      size_t completed_cnt = 0;
      auto const &item_set = dk_dfa_ptr->get_LR_0_item_set(final_state);
      for (auto const &item : item_set.get_kernel_items()) {
        if (item.completed()) {
          completed_cnt++;
          continue;
        }
        if (item.get_grammar_symbal().is_terminal()) {
          return false;
        }
      }
      if (completed_cnt != 1) {
        std::cerr << "completed_cnt is " << completed_cnt << std::endl;
        return false;
      }
      for (auto const &item : item_set.expand_nonkernel_items(*this)) {
        if (item.get_grammar_symbal().is_terminal()) {
          return false;
        }
      }
    }
    return true;
  }
  DPDA DCFG::to_DPDA() const {
    finite_automaton dpda_finite_automaton{{0}, alphabet, {0}, {}};

    auto const &dfa = dk_dfa_ptr->get_dfa();
    std::set<symbol_type> state_symbol_set;
    for (auto const s : dfa.get_states()) {
      state_symbol_set.insert(s);
    }
    auto dk_state_set_alphabet =
        std::make_shared<number_set_alphabet>(state_symbol_set, "dk_state_set");

    DPDA::transition_function_type transition_function;
    auto looping_state = dpda_finite_automaton.add_new_state();
    transition_function[dpda_finite_automaton.get_start_state()][{}] = {
        looping_state, dfa.get_start_state()};

    auto accept_state = dpda_finite_automaton.add_new_state();
    auto goto_table = dk_dfa_ptr->get_goto_table();
    for (auto const dk_state : state_symbol_set) {
      if (!dfa.is_final_state(dk_state)) {
        for (auto const input_symbol : *alphabet) {
          transition_function.check_stack_and_action(
              looping_state, {input_symbol, dk_state},
              {looping_state, goto_table[{dk_state, input_symbol}]},
              dpda_finite_automaton);
        }
        continue;
      }
      auto dk_final_state = dk_state;
      auto const &item_set = dk_dfa_ptr->get_LR_0_item_set(dk_state);
      auto const &item = *item_set.get_completed_items().begin();
      auto const &head = item.get_head();
      auto const &body = item.get_body();

      if (head == get_start_symbol()) {
        transition_function[looping_state][{{}, dk_final_state}] = {
            accept_state};
        continue;
      }

      if (body.empty()) {
        transition_function.check_stack_and_action(
            looping_state, {{}, dk_final_state},
            {looping_state, goto_table[{dk_final_state, head}]},

            dpda_finite_automaton);

      } else {
        auto from_state = looping_state;
        state_type to_state;
        // pop body states from stack
        for (size_t i = 0; i < body.size(); i++) {
          to_state = dpda_finite_automaton.add_new_state();
          if (i == 0) {
            transition_function[from_state][{{}, dk_final_state}] = {to_state};
          } else {
            for (auto dk_state : *dk_state_set_alphabet) {
              transition_function[from_state][{{}, dk_state}] = {to_state};
            }
          }
          from_state = to_state;
        }

        for (auto const dk_state : state_symbol_set) {
          transition_function[from_state][{{}, dk_state}] = {
              looping_state, goto_table[{dk_state, head}]};
        }
      }
    }
    auto reject_state = dpda_finite_automaton.add_new_state();
    for (auto const input_symbol : *alphabet) {
      transition_function[accept_state][{input_symbol}] = {reject_state};
    }
    transition_function.make_reject_state(reject_state, alphabet);
    dpda_finite_automaton.replace_final_states(accept_state);
    return DPDA(dpda_finite_automaton, dk_state_set_alphabet,
                transition_function);
  }
  std::pair<DCFG::collection_type, DCFG::goto_table_type>
  DCFG::get_collection() const {
    return {dk_dfa_ptr->get_LR_0_item_set_collection(),
            dk_dfa_ptr->get_goto_table()};
  }
} // namespace cyy::computation
