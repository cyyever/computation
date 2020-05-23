/*!
 * \file model_transform.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "model_transform.hpp"
#include "../automaton/automaton.hpp"
#include <unordered_map>

namespace cyy::computation {

  CFG NFA_to_CFG(const NFA &nfa) {
    CFG::production_set_type productions;

    auto const state_to_nonterminal = [](finite_automaton::state_type state) {
      return std::string("S") + std::to_string(state);
    };

    for (auto const &[config, next_states] : nfa.get_transition_function()) {
      for (auto const &next_state : next_states) {
        productions[state_to_nonterminal(config.state)].push_back(
            {config.input_symbol, state_to_nonterminal(next_state)});
      }
    }

    for (auto const &[cur_state, next_states] :
         nfa.get_epsilon_transition_function()) {
      for (auto const &next_state : next_states) {
        productions[state_to_nonterminal(cur_state)].push_back(
            {state_to_nonterminal(next_state)});
      }
    }

    for (auto const &final_state : nfa.get_final_states()) {
      productions[state_to_nonterminal(final_state)].emplace_back();
    }

    return {nfa.get_alphabet().get_name(),
            state_to_nonterminal(nfa.get_start_state()), productions};
  }

  PDA CFG_to_PDA(CFG cfg) {
    cfg.eliminate_left_recursion();
    std::unordered_map<grammar_symbol_type, PDA::stack_symbol_type>
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
                         const std::optional<CFG::nonterminal_type> &head,
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
        transition_function[{from_state, {}, get_stack_symbol(*head)}].emplace(
            new_state, new_top);
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

        transition_function[{old_state}].emplace(new_state,
                                                 get_stack_symbol(*it));
        old_state = new_state;
      }
    };
    push_body(start_state, loop_state, {},
              {cfg.get_start_symbol(), cfg.get_alphabet().get_endmarker()});

    for (const auto &[head, bodies] : cfg.get_productions()) {
      for (auto const &body : bodies) {
        push_body(loop_state, loop_state, head, body);
      }
    }

    for (auto const &terminal : cfg.get_terminals()) {
      transition_function[{loop_state, terminal, get_stack_symbol(terminal)}]
          .emplace(loop_state);
    }
    transition_function[{loop_state,
                         {},
                         get_stack_symbol(cfg.get_alphabet().get_endmarker())}]
        .emplace(final_state, std::optional<PDA::stack_symbol_type>{});
    return PDA(std::move(states), cfg.get_alphabet().get_name(), "all",
               start_state, std::move(transition_function), {final_state});
  }

  CFG PDA_to_CFG(PDA pda) {
    pda.normalize_transitions();
    using from_state_type = PDA::state_type;
    using to_state_type = PDA::state_type;
    std::map<PDA::stack_symbol_type,
             std::set<std::tuple<from_state_type,
                                 std::optional<PDA::input_symbol_type>,
                                 to_state_type>>>
        push_stack_trainsitions;

    std::map<PDA::stack_symbol_type,
             std::set<std::tuple<from_state_type,
                                 std::optional<PDA::input_symbol_type>,
                                 to_state_type>>>
        pop_stack_trainsitions;

    for (auto &[situation, actions] : pda.get_transition_function()) {
      auto const &top_symbol = situation.stack_symbol;
      for (auto const &action : actions) {
        if (top_symbol.has_value()) {
          pop_stack_trainsitions[*top_symbol].emplace(
              situation.state, situation.input_symbol, action.state);
        } else if (action.stack_symbol.has_value()) {
          push_stack_trainsitions[*action.stack_symbol].emplace(
              situation.state, situation.input_symbol, action.state);
        }
      }
    }

    auto get_nonterminal = [](from_state_type from_state,
                              to_state_type to_state) {
      return "A_" + std::to_string(from_state) + "_" + std::to_string(to_state);
    };

    CFG::production_set_type productions;
    for (auto const s : pda.get_states()) {
      productions[get_nonterminal(s, s)] = {{}};
    }

    for (auto const &[s, prev_step] : push_stack_trainsitions) {
      auto it = pop_stack_trainsitions.find(s);
      if (it == pop_stack_trainsitions.end()) {
        continue;
      }

      for (auto const &[prev_from_state, prev_input, prev_to_state] :
           prev_step) {
        for (auto const &[next_from_state, next_input, next_to_state] :
             it->second) {

          CFG_production::body_type body;

          if (prev_input.has_value()) {
            body.emplace_back(*prev_input);
          }
          body.emplace_back(get_nonterminal(prev_to_state, next_from_state));

          if (next_input.has_value()) {
            body.emplace_back(*next_input);
          }
          productions[get_nonterminal(prev_from_state, next_to_state)]
              .emplace_back(std::move(body));
        }
      }
    }
    for (auto const a : pda.get_states()) {
      for (auto const b : pda.get_states()) {
        for (auto const c : pda.get_states()) {
          productions[get_nonterminal(a, c)].emplace_back(
              CFG_production::body_type{get_nonterminal(a, b),
                                        get_nonterminal(b, c)});
        }
      }
    }
    return CFG(
        pda.get_alphabet().get_name(),
        get_nonterminal(pda.get_start_state(), *pda.get_final_states().begin()),
        productions);
  }
} // namespace cyy::computation
