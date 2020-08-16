#include <iostream>
#include <ranges>

#include "dcfg.hpp"
#include "lang/number_set_alphabet.hpp"
namespace cyy::computation {
  DCFG::DCFG(std::shared_ptr<ALPHABET> alphabet_,
             nonterminal_type start_symbol_, production_set_type productions_)

      : LR_grammar(alphabet_, start_symbol_, std::move(productions_)),
        dk_dfa_ptr(std::make_shared<DK_DFA>(*this)) {
    if (!DK_test()) {
      throw exception::no_DCFG("DK test failed");
    }
  }

  bool DCFG::DK_test() const {
    for (auto final_state : dk_dfa_ptr->get_dfa().get_final_states()) {
      size_t completed_cnt = 0;
      auto const &item_set = dk_dfa_ptr->get_LR_0_item_set(final_state);
      for (auto const &kernel_item : item_set.get_kernel_items()) {
        if (kernel_item.completed()) {
          completed_cnt++;
          continue;
        }
        if (kernel_item.get_grammar_symbal().is_terminal()) {
          return false;
        }
      }
      if (completed_cnt != 1) {
        std::cerr << "completed_cnt is " << completed_cnt << std::endl;
        return false;
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
    ALPHABET::set(dk_state_set_alphabet);

    DPDA::transition_function_type transition_function;
    auto looping_state = dpda_finite_automaton.add_new_state();
    transition_function[dpda_finite_automaton.get_start_state()][{}] = {
        looping_state, dfa.get_start_state()};

    for (auto const dk_state : state_symbol_set) {
      if (dfa.is_final_state(dk_state)) {
        continue;
      }
      for (auto const input_symbol : *alphabet) {
        transition_function.check_stack_and_action(
            looping_state, {input_symbol, dk_state},
            {looping_state, dfa.get_transition_function()
                                .find({dk_state, input_symbol})
                                ->second},
            dpda_finite_automaton);
      }
    }
    auto accept_state = dpda_finite_automaton.add_new_state();
    auto goto_table = dk_dfa_ptr->get_goto_table();
    for (auto dk_final_state : dfa.get_final_states()) {
      auto const &item_set = dk_dfa_ptr->get_LR_0_item_set(dk_final_state);
      auto const &kernel_item = *item_set.get_completed_items().begin();
      auto const &head = kernel_item.get_head();
      auto const &body = kernel_item.get_body();

      if (head == get_start_symbol()) {
        transition_function[looping_state][{{}, dk_final_state}] = {
            accept_state};
        continue;
      }

      if (body.empty()) {
        auto pop_state = dpda_finite_automaton.add_new_state();
        transition_function[looping_state][{{}, dk_final_state}] = {
            pop_state, dk_final_state};
        transition_function[pop_state][{}] = {
            looping_state, goto_table[{dk_final_state, head}]};
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
          transition_function[to_state][{{}, dk_state}] = {
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
    return DPDA(dpda_finite_automaton, dk_state_set_alphabet->get_name(),
                transition_function);
  }
  void DCFG::construct_parsing_table() const {
    /*
    const_cast<DCFG *>(this)->normalize_start_symbol();
    auto const &collection = dk_dfa_ptr->get_LR_0_item_set_collection();
    goto_table=dk_dfa_ptr->get_goto_table();

    for (auto const &[p, next_state] : goto_transitions) {
      auto ptr = p.second.get_terminal_ptr();
      if (ptr) {
      } else {
        assert(p.second.get_terminal() != ALPHABET::endmarker);
        action_table[{p.first, p.second.get_terminal()}] = next_state;
      }
    }
    for (auto const &[state, set] : collection) {
      for (const auto &[kernel_item, lookahead_set] :
           set.get_completed_items()) {

        for (const auto &lookahead : lookahead_set) {
          // conflict
          auto it = action_table.find({state, lookahead});
          if (it != action_table.end()) {
            std::ostringstream os;
            os << "state " << state << " with head " << kernel_item.get_head()
               << " conflict with follow terminal "
               << alphabet->to_string(lookahead) << " and action index "
               << it->second.index();
            throw cyy::computation::exception::no_LR_1_grammar(os.str());
          }
          if (lookahead == ALPHABET::endmarker &&
              kernel_item.get_head() == get_start_symbol()) {
            assert(lookahead_set.size() == 1);
            action_table[{state, lookahead}] = true;
          } else {
            action_table[{state, lookahead}] = kernel_item.get_production();
          }
        }
      }
    }
    const_cast<DCFG *>(this)->remove_head(get_start_symbol());
    */
  }
} // namespace cyy::computation
