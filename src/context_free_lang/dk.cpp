/*!
 * \file dk.cpp
 *
 */

#include "dk.hpp"
#include "cfg.hpp"
#include "lang/range_alphabet.hpp"
#include "lang/union_alphabet.hpp"
#include "regular_lang/nfa.hpp"

namespace cyy::computation {
  DK_DFA::DK_DFA(const CFG &cfg) {

    auto max_symbol = cfg.get_alphabet().get_max_symbol();
    auto nonterminals = cfg.get_nonterminals();

    alphabet_of_nonterminals = cfg.get_nonterminal_alphabet();

    using state_set_type = NFA::state_set_type;

    std::unordered_map<LR_0_item, state_type> item_to_nfa_state_map;
    std::unordered_map<state_type, LR_0_item> NFA_state_to_item_map;

    NFA nfa{{0}, cfg.get_full_alphabet(), 0, {}, {}};
    auto item_to_nfa_state = [&item_to_nfa_state_map, &NFA_state_to_item_map,
                              &nfa](const LR_0_item &item) {
      auto it = item_to_nfa_state_map.find(item);
      if (it == item_to_nfa_state_map.end()) {
        auto new_state = nfa.add_new_state();
        it = item_to_nfa_state_map.try_emplace(item, new_state).first;
        NFA_state_to_item_map.emplace(new_state, item);
      }
      return it->second;
    };

    std::unordered_map<CFG::nonterminal_type, state_set_type> head_states;
    for (auto const &[head, bodies] : cfg.get_productions()) {
      for (auto const &body : bodies) {
        cyy::computation::LR_0_item init_item(head, body);
        auto state = item_to_nfa_state(init_item);
        if (head == cfg.get_start_symbol()) {
          nfa.add_epsilon_transition(nfa.get_start_state(), {state});
        }
        head_states[head].insert(state);
      }
    }

    for (auto const &[head, bodies] : cfg.get_productions()) {
      for (auto const &body : bodies) {
        cyy::computation::LR_0_item cur_item(head, body);

        for (auto const &grammar_symbol : body) {
          auto next_item = cur_item;
          next_item.go();
          symbol_type symbol;
          auto cur_state = item_to_nfa_state(cur_item);
          if (grammar_symbol.is_terminal()) {
            symbol = grammar_symbol.get_terminal();
          } else {
            nfa.add_epsilon_transition(
                cur_state,
                state_set_type(head_states[grammar_symbol.get_nonterminal()]));
            symbol = alphabet_of_nonterminals->get_symbol(
                grammar_symbol.get_nonterminal());
          }
          nfa.add_transition({cur_state, symbol},
                             {item_to_nfa_state(next_item)});
          cur_item = next_item;
        }
        assert(cur_item.completed());
        nfa.add_final_state(item_to_nfa_state(cur_item));
      }
    }

    auto [dfa, dfa_to_nfa_state_map] = nfa.to_DFA_with_mapping();
    for (auto const &[dfa_state, nfa_state_set] : dfa_to_nfa_state_map) {
      auto &item_set = collection[dfa_state];
      for (auto const &nfa_state : nfa_state_set) {
        auto it = NFA_state_to_item_map.find(nfa_state);
        if (it != NFA_state_to_item_map.end()) {
          item_set.add_item(it->second);
        }
      }
    }
    dfa_ptr = std::make_shared<DFA>(std::move(dfa));
  }

  DK_DFA::goto_table_type DK_DFA::get_goto_table(bool skip_fail_state) const {
    goto_table_type goto_table;
    for (auto const &[situation, next_state] :
         dfa_ptr->get_transition_function()) {
      assert(collection.contains(next_state));
      if (skip_fail_state) {
        if (collection.find(situation.state)->second.empty()) {
          continue;
        }
        if (collection.find(next_state)->second.empty()) {
          continue;
        }
      }
      if (alphabet_of_nonterminals->contain(situation.input_symbol)) {
        goto_table[{situation.state, alphabet_of_nonterminals->get_data(
                                         situation.input_symbol)}] = next_state;
      } else {
        goto_table[{situation.state, situation.input_symbol}] = next_state;
      }
    }
    return goto_table;
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
