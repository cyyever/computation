#include "dcfg.hpp"
#include "lang/set_alphabet.hpp"
#include <ranges>

namespace cyy::computation {
  bool DCFG::DK_test() const {

    std::tie(dk_opt, nonterminal_to_symbol, state_to_LR_0_item_set) = get_DK();
    for (auto final_state : dk_opt->get_final_states()) {
      size_t completed_cnt = 0;
      auto const &item_set = state_to_LR_0_item_set[final_state];
      for (auto const &kernel_item : item_set.get_kernel_items()) {
        if (kernel_item.completed()) {
          completed_cnt++;
        }
        if (kernel_item.get_grammar_symbal().is_terminal()) {
          return false;
        }
      }
      if (completed_cnt != 1) {
        return false;
      }
    }
    return true;
  }
  void DCFG::to_DPDA() const {
    using state_type = DPDA::state_type;
    finite_automaton dpda_finite_automaton{{0}, alphabet->get_name(), {0}, {}};

    std::set<symbol_type> state_symbol_set;
    for (auto const s : dk_opt->get_states()) {
      state_symbol_set.insert(s);
    }
    auto dk_state_set_alphabet =
        std::make_shared<set_alphabet>(state_symbol_set, "dk_state_set");
    ALPHABET::set(dk_state_set_alphabet);

    DPDA::transition_function_type transition_function;
    auto looping_state = dpda_finite_automaton.add_new_state();
    transition_function[dpda_finite_automaton.get_start_state()][{}] = {
        looping_state, dk_opt->get_start_state()};

    for (auto const dk_state : state_symbol_set) {
      if (dk_opt->is_final_state(dk_state)) {
        continue;
      }
      for (auto const input_symbol : *alphabet) {
        transition_function[looping_state][{input_symbol, dk_state}] = {
            looping_state, dk_opt->get_transition_function()
                               .find({dk_state, input_symbol})
                               ->second};
      }
      /* for (auto const &[situation, next_state] : */
      /*      dk_opt->get_transition_function()) { */
      /*   if (!symbol_to_nonterminal.contains(situation.input_symbol)) { */
      /*   } */
      /* } */
    }
    for (auto dk_final_state : dk_opt->get_final_states()) {
      auto const &item_set = state_to_LR_0_item_set[dk_final_state];
      for (auto const &kernel_item : item_set.get_kernel_items()) {
        if (!kernel_item.completed()) {
          break;
        }
        auto const &head = kernel_item.get_head();
        auto const &body = kernel_item.get_body();
        state_type pop_state;
        if (body.empty()) {
          pop_state = looping_state;
        } else {
          pop_state = looping_state;
        }
        transition_function[pop_state][{{}, dk_final_state}] = {
            looping_state,
            dk_opt->get_transition_function()
                .find({dk_final_state, nonterminal_to_symbol[head]})
                ->second};
      }
    }
  }
} // namespace cyy::computation
