#include "dcfg.hpp"

#include "alphabet/number_set_alphabet.hpp"

namespace cyy::computation {
  DCFG::DCFG(ALPHABET_ptr alphabet_, nonterminal_type start_symbol_,
             production_set_type productions_)

      : LR_0_grammar(alphabet_, start_symbol_, std::move(productions_)),
        dk_dfa_opt(std::make_optional<DK_DFA>(*this)) {
    if (!DK_test()) {
      throw exception::no_DCFG("DK test failed");
    }
  }

  bool DCFG::DK_test() const {
    for (auto &[_, item_set] : dk_dfa_opt->get_LR_0_item_set_collection()) {
      if (!item_set.has_completed_items()) {
        continue;
      }
      size_t completed_cnt = 0;
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
    finite_automaton dpda_finite_automaton{{0}, alphabet, 0, {}};

    auto const &dfa = dk_dfa_opt->get_dfa();
    auto state_symbol_set = dfa.get_state_symbol_set();
    auto dk_state_set_alphabet =
        std::make_shared<number_set_alphabet>(state_symbol_set, "dk_state_set");

    DPDA::transition_function_type transition_function;
    auto looping_state = dpda_finite_automaton.add_new_state();
    transition_function[dpda_finite_automaton.get_start_state()][{}] = {
        looping_state, dfa.get_start_state()};

    auto accept_state = dpda_finite_automaton.add_new_state();
    auto goto_table = get_goto_table();
    for (auto const dk_state : state_symbol_set) {
      // shift
      if (!dfa.is_final_state(dk_state)) {
        for (auto const input_symbol : alphabet->get_view()) {
          for (auto from_state : {looping_state, accept_state}) {
            transition_function.check_stack_and_action(
                from_state, {input_symbol, dk_state},
                {looping_state, goto_table[{dk_state, input_symbol}]},
                dpda_finite_automaton);
          }
        }
        continue;
      }
      // reduce
      auto dk_final_state = dk_state;
      auto completed_items =
          dk_dfa_opt->get_LR_0_item_set(dk_state).get_completed_items();
      /* assert(std::ranges::size(completed_items) == 1); */

      auto const &item = *(completed_items.begin());
      auto const &head = item.get_head();
      auto const &body = item.get_body();

      for (auto from_state : {looping_state, accept_state}) {
        // pop body states from stack
        for (size_t i = 0; i < body.size(); i++) {
          auto to_state = dpda_finite_automaton.add_new_state();

          if (i == 0) {
            transition_function[from_state][{{}, dk_final_state}] = {to_state};
          } else {
            transition_function.pop_stack_and_action(from_state, {to_state},
                                                     *dk_state_set_alphabet);
          }
          from_state = to_state;
        }

        for (auto const prev_dk_state : state_symbol_set) {
          if (body.empty() && prev_dk_state != dk_final_state) {
            continue;
          }
          auto destination_state = looping_state;
          if (prev_dk_state == dfa.get_start_state() &&
              head == get_start_symbol()) {
            destination_state = accept_state;
          }
          transition_function.check_stack_and_action(
              from_state, {{}, prev_dk_state},
              {destination_state, goto_table[{prev_dk_state, head}]},
              dpda_finite_automaton);
        }
      }
    }
    dpda_finite_automaton.replace_final_states(accept_state);
    return {dpda_finite_automaton, dk_state_set_alphabet,
                transition_function};
  }
  std::pair<DCFG::collection_type, DCFG::goto_table_type>
  DCFG::get_collection() const {
    return {dk_dfa_opt->get_LR_0_item_set_collection(),
            dk_dfa_opt->get_goto_table()};
  }
} // namespace cyy::computation
