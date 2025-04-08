/*!
 * \file dk.cpp
 *
 */

#include <cyy/algorithm/hash.hpp>
#include "dk.hpp"
// #include "cfg.hpp"
// #include "regular_lang/nfa.hpp"

namespace cyy::computation {
  DK_DFA::DK_DFA(const CFG &cfg) : DK_DFA_base(cfg) {
    using state_set_type = NFA::state_set_type;

    std::unordered_map<LR_0_item, state_type> item_to_nfa_state_map;

    NFA nfa{{0}, cfg.get_full_alphabet(), 0, {}, {}};
    auto item_to_nfa_state = [&item_to_nfa_state_map,
                              &nfa](const LR_0_item &item) {
      auto it = item_to_nfa_state_map.find(item);
      if (it == item_to_nfa_state_map.end()) {
        const auto new_state = nfa.add_new_state();
        item_to_nfa_state_map.emplace(item, new_state);
        return new_state;
      }
      return it->second;
    };

    std::unordered_map<CFG::nonterminal_type, state_set_type> head_states;
    for (auto const &[head, bodies] : cfg.get_productions()) {
      for (auto const &body : bodies) {
        cyy::computation::LR_0_item const init_item(head, body);
        auto state = item_to_nfa_state(init_item);
        if (head == cfg.get_start_symbol()) {
          nfa.add_epsilon_transition(nfa.get_start_state(), {state});
        }
        head_states[head].emplace(state);
      }
    }

    for (auto const &[head, bodies] : cfg.get_productions()) {
      for (auto const &body : bodies) {
        cyy::computation::LR_0_item cur_item(head, body);

        for (auto const &grammar_symbol : body) {
          auto next_item = cur_item;
          next_item.go();
          NFA::input_symbol_type symbol{};
          const auto cur_state = item_to_nfa_state(cur_item);
          if (grammar_symbol.is_terminal()) {
            symbol = grammar_symbol.get_terminal();
          } else {
            nfa.add_epsilon_transition(
                cur_state,
                state_set_type(head_states[grammar_symbol.get_nonterminal()]));
            symbol = alphabet_of_nonterminals->get_symbol(
                grammar_symbol.get_nonterminal());
          }
          nfa.add_transition({.state = cur_state, .input_symbol = symbol},
                             {item_to_nfa_state(next_item)});
          cur_item = next_item;
        }
        assert(cur_item.completed());
        nfa.add_final_state(item_to_nfa_state(cur_item));
      }
    }
    std::unordered_map<state_type, LR_0_item> nfa_state_to_item_map;
    for (auto &[k, v] : std::move(item_to_nfa_state_map)) {
      nfa_state_to_item_map.emplace(std::move(v), std::move(k));
    }

    auto [dfa, dfa_to_nfa_state_map] = nfa.to_DFA_with_mapping();
    for (auto const &[nfa_state_set, dfa_state] : dfa_to_nfa_state_map) {
      auto &item_set = collection[dfa_state];
      for (auto const &nfa_state : nfa_state_set) {
        auto it = nfa_state_to_item_map.find(nfa_state);
        if (it != nfa_state_to_item_map.end()) {
          item_set.add_item(it->second);
        }
      }
    }
    dfa_ptr = std::make_shared<DFA>(std::move(dfa));
  }

  const LR_0_item_set &DK_DFA::get_LR_0_item_set(state_type state) const {
    assert(collection.contains(state));
    auto it = collection.find(state);
    if (it == collection.end()) {
      throw std::runtime_error("invalid state");
    }
    return it->second;
  }
  std::string DK_DFA::MMA_draw(const CFG &cfg) const {
    std::string cmd = "TableForm[{";
    for (auto const &[state, set] : collection) {
      cmd += std::to_string(state) + "->" + set.MMA_draw(cfg);
      cmd.push_back(',');
    }
    auto dfa_cmd = dfa_ptr->MMA_draw();
    cmd += dfa_cmd + "}]";
    return cmd;
  }

} // namespace cyy::computation
