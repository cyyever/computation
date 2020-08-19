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
      auto const &item = *item_set.get_completed_items().begin();
      auto const &head = item.get_head();
      auto const &body = item.get_body();

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
  void DCFG::construct_parsing_table() const {
    CFG augmented_cfg(*this);
    augmented_cfg.normalize_start_symbol();
    DK_DFA augmented_dk_dfa(augmented_cfg);

    auto const &collection = augmented_dk_dfa.get_LR_0_item_set_collection();
    goto_table = augmented_dk_dfa.get_goto_table();

    for (auto const &[p, next_state] : goto_table) {
      auto const &[from_state, grammar_symbol] = p;
      if (grammar_symbol.is_terminal()) {
        if (grammar_symbol.get_terminal() != ALPHABET::endmarker) {
          action_table[{from_state, grammar_symbol.get_terminal()}] =
              next_state;
        }
      }
    }

    auto terminals = get_terminals();
    terminals.insert(ALPHABET::endmarker);
    for (auto const &[state, set] : collection) {
      for (const auto &item : set.get_completed_items()) {
        for (auto lookahead : terminals) {
          if (item.get_head() == augmented_cfg.get_start_symbol()) {
            lookahead = ALPHABET::endmarker;
          }
          // conflict
          auto it = action_table.find({state, lookahead});
          if (it != action_table.end()) {
            std::ostringstream os;
            os << "state " << state << " with head " << item.get_head()
               << " conflict with follow terminal "
               << alphabet->to_string(lookahead) << " and action index "
               << it->second.index();
            throw cyy::computation::exception::no_LR_grammar(os.str());
          }
          if (item.get_head() == augmented_cfg.get_start_symbol()) {
            action_table[{state, lookahead}] = true;
            break;
          } else {
            action_table[{state, lookahead}] = item.get_production();
          }
        }
      }
    }
  }
} // namespace cyy::computation
