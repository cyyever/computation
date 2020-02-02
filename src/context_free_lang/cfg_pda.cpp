/*!
 * \file cfg_PDA.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "cfg.hpp"

namespace cyy::computation {

  PDA CFG::to_PDA() const {
    std::map<grammar_symbol_type, PDA::stack_symbol_type>
        grammar_symbol_to_stack_symbol;
    PDA::stack_symbol_type next_stack_symbol = 0;
    auto get_stack_symbol =
        [&grammar_symbol_to_stack_symbol, &next_stack_symbol ](
            const grammar_symbol_type &grammar_symbol) -> auto {
      auto it = grammar_symbol_to_stack_symbol.find(grammar_symbol);
      if (it != grammar_symbol_to_stack_symbol.end()) {
        return it->second;
      }
      auto stack_symbol = next_stack_symbol;
      next_stack_symbol++;
      grammar_symbol_to_stack_symbol[grammar_symbol] = stack_symbol;
      return stack_symbol;
    };

    PDA::state_type start_state = 0;
    PDA::state_type loop_state = 1;
    PDA::state_type final_state = 2;
    std::set<PDA::state_type> states{start_state, loop_state, final_state};
    PDA::transition_function_type transition_function;
    auto push_body = [&states, &get_stack_symbol, &transition_function](
                         PDA::state_type from_state, PDA::state_type to_state,
                         const std::optional<nonterminal_type> &head,
                         const CFG_production::body_type &body) {
      PDA::state_type new_state, old_state;
      auto it = body.rbegin();
      if (head.has_value()) {
        if (body.size() <= 1) {
          new_state = to_state;
        } else {
          new_state = *states.rbegin() + 1;
          states.insert(new_state);
        }
        std::optional<PDA::stack_symbol_type> new_top;
        if (it != body.rend()) {
          new_top = get_stack_symbol(*it);
          it++;
        }
        transition_function[{{}, from_state, get_stack_symbol(*head)}].insert(
            {new_state, new_top});
        old_state = new_state;
      } else {
        old_state = from_state;
      }

      for (; it != body.rend(); it++) {
        if (it + 1 == body.rend()) {
          new_state = to_state;
        } else {
          new_state = *states.rbegin() + 1;
          states.insert(new_state);
        }

        transition_function[{{}, old_state, {}}].insert(
            {new_state, get_stack_symbol(*it)});
        old_state = new_state;
      }
    };
    push_body(start_state, loop_state, {},
              {start_symbol, alphabet->get_endmarker()});

    for (const auto &[head, bodies] : productions) {
      for (auto const &body : bodies) {
        push_body(loop_state, loop_state, head, body);
      }
    }

    for (auto const &terminal : get_terminals()) {
      transition_function[{terminal, loop_state, get_stack_symbol(terminal)}]
          .insert({loop_state, {}});
    }
    transition_function[{{},
                         loop_state,
                         get_stack_symbol(alphabet->get_endmarker())}]
        .insert({final_state, {}});
    return PDA(std::move(states), alphabet->get_name(), "all", start_state,
               std::move(transition_function), {final_state});
  }

} // namespace cyy::computation
