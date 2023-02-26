/*!
 * \file model_transform.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "model_transform.hpp"

#include <unordered_map>
#include <vector>

#include "automaton/automaton.hpp"
#include "cfg_production.hpp"

namespace cyy::computation {

  CFG NFA_to_CFG(const NFA &nfa) {
    CFG::production_set_type productions;

    auto const state_to_nonterminal = [](finite_automaton::state_type state) {
      return std::string("S") + std::to_string(state);
    };

    for (auto const &[config, next_states] : nfa.get_transition_function()) {
      for (auto const &next_state : next_states) {
        productions[state_to_nonterminal(config.state)].insert(
            {config.input_symbol, state_to_nonterminal(next_state)});
      }
    }

    for (auto const &[cur_state, next_states] :
         nfa.get_epsilon_transition_function()) {
      for (auto const &next_state : next_states) {
        productions[state_to_nonterminal(cur_state)].insert(
            {state_to_nonterminal(next_state)});
      }
    }

    for (auto const &final_state : nfa.get_final_states()) {
      productions[state_to_nonterminal(final_state)].emplace();
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
        [&grammar_symbol_to_stack_symbol, &
         next_stack_symbol ](const grammar_symbol_type &grammar_symbol) -> auto{
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
    PDA::state_set_type states{start_state, loop_state, final_state};
    finite_automaton dpda_automaton(states, cfg.get_alphabet_ptr(), start_state,
                                    {final_state});
    PDA::transition_function_type transition_function;
    auto push_body = [&dpda_automaton, &get_stack_symbol, &transition_function](
                         PDA::state_type from_state, PDA::state_type to_state,
                         const std::optional<CFG::nonterminal_type> &head,
                         const CFG_production::body_type &body) {
      PDA::state_type new_state, old_state;
      auto it = body.rbegin();
      if (head.has_value()) {
        if (body.size() <= 1) {
          new_state = to_state;
        } else {
          new_state = dpda_automaton.add_new_state();
        }
        std::optional<PDA::stack_symbol_type> new_top;
        if (it != body.rend()) {
          new_top = get_stack_symbol(*it);
          ++it;
        }
        transition_function[{from_state, {}, get_stack_symbol(*head)}].emplace(
            new_state, new_top);
        old_state = new_state;
      } else {
        old_state = from_state;
      }

      for (; it != body.rend(); ++it) {
        if (it + 1 == body.rend()) {
          new_state = to_state;
        } else {
          new_state = dpda_automaton.add_new_state();
        }

        transition_function[{old_state}].emplace(new_state,
                                                 get_stack_symbol(*it));
        old_state = new_state;
      }
    };
    push_body(start_state, loop_state, {},
              {cfg.get_start_symbol(), ALPHABET::endmarker});

    for (const auto &[head, bodies] : cfg.get_productions()) {
      for (auto const &body : bodies) {
        push_body(loop_state, loop_state, head, body);
      }
    }

    for (auto const &terminal : cfg.get_terminals()) {
      transition_function[{loop_state, terminal, get_stack_symbol(terminal)}]
          .emplace(loop_state);
    }
    transition_function[{loop_state, {}, get_stack_symbol(ALPHABET::endmarker)}]
        .emplace(final_state, std::optional<PDA::stack_symbol_type>{});
    return {std::move(dpda_automaton), cfg.get_full_alphabet(),
               std::move(transition_function)};
  }

  CFG PDA_to_CFG(PDA pda) {
    pda.prepare_CFG_conversion();
    using from_state_type = PDA::state_type;
    using to_state_type = PDA::state_type;
    std::unordered_map<
        PDA::stack_symbol_type,
        std::vector<std::tuple<PDA::situation_type, to_state_type>>>
        push_stack_transitions;

    std::unordered_map<
        PDA::stack_symbol_type,
        std::vector<std::tuple<PDA::situation_type, to_state_type>>>
        pop_stack_transitions;

    for (auto &[situation, actions] : pda.get_transition_function()) {
      auto const &top_symbol = situation.stack_symbol;
      for (auto const &action : actions) {
        if (top_symbol.has_value()) {
          pop_stack_transitions[*top_symbol].emplace_back(situation,
                                                          action.state);
          continue;
        }
        push_stack_transitions[*action.stack_symbol].emplace_back(situation,
                                                                  action.state);
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

    for (auto const &[s, prev_step] : push_stack_transitions) {
      auto it = pop_stack_transitions.find(s);
      if (it == pop_stack_transitions.end()) {
        continue;
      }

      for (auto const &[prev_situation, prev_to_state] : prev_step) {
        for (auto const &[next_situation, next_to_state] : it->second) {

          CFG_production::body_type body;

          if (prev_situation.use_input()) {
            body.emplace_back(prev_situation.get_input());
          }
          body.emplace_back(
              get_nonterminal(prev_to_state, next_situation.state));

          if (next_situation.use_input()) {
            body.emplace_back(next_situation.get_input());
          }
          productions[get_nonterminal(prev_situation.state, next_to_state)]
              .emplace(std::move(body));
        }
      }
    }
    for (auto const a : pda.get_states()) {
      for (auto const b : pda.get_states()) {
        for (auto const c : pda.get_states()) {
          productions[get_nonterminal(a, c)].emplace(CFG_production::body_type{
              get_nonterminal(a, b), get_nonterminal(b, c)});
        }
      }
    }
    return {
        pda.get_alphabet_ptr(),
        get_nonterminal(pda.get_start_state(), *pda.get_final_states().begin()),
        std::move(productions)};
  }
  DCFG DPDA_to_DCFG(endmarked_DPDA dpda) {
    dpda.prepare_DCFG_conversion();
    using from_state_type = endmarked_DPDA::state_type;
    using to_state_type = endmarked_DPDA::state_type;
    using input_symbol_type = endmarked_DPDA::input_symbol_type;
    using stack_symbol_type = endmarked_DPDA::stack_symbol_type;

    std::unordered_map<
        stack_symbol_type,
        std::set<std::tuple<from_state_type, std::optional<input_symbol_type>,
                            to_state_type>>>
        push_stack_transitions;

    std::unordered_map<
        stack_symbol_type,
        std::set<std::tuple<from_state_type, std::optional<input_symbol_type>,
                            to_state_type>>>
        pop_stack_transitions;

    for (auto &[from_state, transfers] : dpda.get_transition_function()) {
      for (const auto &[situation, action] : transfers) {
        auto next_state = action.state;
        auto const &top_symbol = situation.stack_symbol;
        // pop
        if (top_symbol.has_value()) {
          pop_stack_transitions[*top_symbol].emplace(
              from_state, situation.input_symbol, next_state);
          continue;
        }
        // push
        push_stack_transitions[*action.stack_symbol].emplace(
            from_state, situation.input_symbol, next_state);
      }
    }

    auto get_nonterminal = [](from_state_type from_state,
                              to_state_type to_state) {
      return "A_" + std::to_string(from_state) + "_" + std::to_string(to_state);
    };

    CFG::production_set_type productions;
    for (auto const s : dpda.get_states()) {
      productions[get_nonterminal(s, s)] = {{}};
    }

    for (auto const &[stack_symbol, prev_step] : push_stack_transitions) {
      auto it = pop_stack_transitions.find(stack_symbol);
      if (it == pop_stack_transitions.end()) {
        continue;
      }

      for (auto const &[prev_from_state, prev_input, prev_to_state] :
           prev_step) {
        for (auto const &[next_from_state, next_input, next_to_state] :
             it->second) {
          for (auto state : dpda.get_states()) {
            auto head = get_nonterminal(state, next_to_state);
            CFG_production::body_type body;
            body.emplace_back(get_nonterminal(state, prev_from_state));

            if (prev_input.has_value()) {
              body.emplace_back(*prev_input);
            }
            body.emplace_back(get_nonterminal(prev_to_state, next_from_state));
            if (next_input.has_value()) {
              body.emplace_back(*next_input);
            }
            productions[head].emplace(std::move(body));
          }
        }
      }
    }
    assert(dpda.get_final_states().size() == 1);
    return {dpda.get_alphabet_ptr(),
                get_nonterminal(dpda.get_start_state(),
                                *dpda.get_final_states().begin()),
                std::move(productions)};
  }
} // namespace cyy::computation
