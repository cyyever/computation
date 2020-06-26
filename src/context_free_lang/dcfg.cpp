#include <ranges>

#include "dcfg.hpp"
#include "lang/number_set_alphabet.hpp"
namespace cyy::computation {
  bool DCFG::DK_test() const {

    std::tie(dk_opt, nonterminal_to_symbol, state_to_LR_0_item_set) = get_DK();
    for (auto final_state : dk_opt->get_final_states()) {
      size_t completed_cnt = 0;
      assert(state_to_LR_0_item_set.contains(final_state));
      auto const &item_set = state_to_LR_0_item_set[final_state];
      for (auto const &kernel_item : item_set.get_kernel_items()) {
        if (kernel_item.completed()) {
          completed_cnt++;
          continue;
        }
        if (kernel_item.get_grammar_symbal().is_terminal()) {
          return false;
        }
      }
      for (auto const &nonkernel_item : item_set.get_nonkernel_items()) {
        auto it = productions.find(nonkernel_item);
        assert(it != productions.end());
        completed_cnt += std::ranges::count_if(
            it->second, [](auto const &body) { return body.empty(); });
      }
      if (completed_cnt != 1) {
        printf("cnt %zu\n", completed_cnt);
        return false;
      }
    }
    return true;
  }
  DPDA DCFG::to_DPDA() const {
    using state_type = DPDA::state_type;
    finite_automaton dpda_finite_automaton{{0}, alphabet, {0}, {}};

    std::set<symbol_type> state_symbol_set;
    for (auto const s : dk_opt->get_states()) {
      state_symbol_set.insert(s);
    }
    auto dk_state_set_alphabet =
        std::make_shared<number_set_alphabet>(state_symbol_set, "dk_state_set");
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
        transition_function.check_stack_and_action(
            looping_state, {input_symbol, dk_state},
            {looping_state, dk_opt->get_transition_function()
                                .find({dk_state, input_symbol})
                                ->second},
            dpda_finite_automaton);
      }
    }
    auto accept_state = dpda_finite_automaton.add_new_state();
    for (auto dk_final_state : dk_opt->get_final_states()) {
      auto const &item_set = state_to_LR_0_item_set[dk_final_state];
      auto const &kernel_item = *item_set.get_completed_items().begin();
      auto const &head = kernel_item.get_head();
      auto const &body = kernel_item.get_body();

      // TODO use new start symbol
      if (head == start_symbol) {
        transition_function[looping_state][{{}, dk_final_state}] = {
            accept_state};
        continue;
      }

      if (body.empty()) {
        auto pop_state = dpda_finite_automaton.add_new_state();
        transition_function[looping_state][{{}, dk_final_state}] = {
            pop_state, dk_final_state};
        transition_function[pop_state][{}] = {
            looping_state,
            dk_opt->get_transition_function()
                .find({dk_final_state, nonterminal_to_symbol[head]})
                ->second};
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
              looping_state, dk_opt->get_transition_function()
                                 .find({dk_state, nonterminal_to_symbol[head]})
                                 ->second};
        }
      }
    }
    auto reject_state = dpda_finite_automaton.add_new_state();
    for (auto const input_symbol : *alphabet) {
      transition_function[accept_state][{input_symbol}] = {reject_state};
    }
    transition_function.make_reject_state(reject_state,alphabet);
    dpda_finite_automaton.replace_final_states(accept_state);
    return DPDA(dpda_finite_automaton, dk_state_set_alphabet->get_name(),
                transition_function);
  }
} // namespace cyy::computation
