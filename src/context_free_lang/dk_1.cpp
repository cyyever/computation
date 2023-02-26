/*!
 * \file dk_1.cpp
 */

#include "dk_1.hpp"

#include "regular_lang/nfa.hpp"

namespace cyy::computation {
  DK_1_DFA::DK_1_DFA(const CFG &cfg) : DK_DFA_base(cfg) {

    std::unordered_map<LR_1_item, state_type> item_to_nfa_state_map;
    std::unordered_map<state_type, LR_1_item> NFA_state_to_item_map;

    NFA nfa{{0}, cfg.get_full_alphabet(), 0, {}, {}};
    auto item_to_nfa_state = [&item_to_nfa_state_map, &NFA_state_to_item_map,
                              &nfa](const LR_1_item &item) {
      auto it = item_to_nfa_state_map.find(item);
      if (it == item_to_nfa_state_map.end()) {
        auto new_state = nfa.add_new_state();
        it = item_to_nfa_state_map.try_emplace(item, new_state).first;
        NFA_state_to_item_map.emplace(new_state, item);
      }
      return it->second;
    };

    CFG::terminal_set_type init_follows = {ALPHABET::endmarker};

    // begin from start symbol
    auto const &start_symbol = cfg.get_start_symbol();
    for (auto const &body : cfg.get_productions().at(start_symbol)) {
      LR_1_item init_item(LR_0_item{start_symbol, body}, init_follows);
      auto state = item_to_nfa_state(init_item);
      nfa.add_epsilon_transition(nfa.get_start_state(), {state});
    }

    for (state_type cur_state = 1; cur_state <= nfa.get_max_state();
         cur_state++) {
      auto it = NFA_state_to_item_map.find(cur_state);
      assert(it != NFA_state_to_item_map.end());
      auto const &cur_item = it->second;
      assert(!cur_item.get_lookahead_symbols().empty());
      if (cur_item.completed()) {
        nfa.add_final_state(cur_state);
        continue;
      }

      symbol_type symbol;
      auto grammar_symbol = cur_item.get_grammar_symbal();
      if (grammar_symbol.is_terminal()) {
        symbol = grammar_symbol.get_terminal();
      } else {
        auto const &head = grammar_symbol.get_nonterminal();
        symbol = alphabet_of_nonterminals->get_symbol(head);
        auto follow_set = cur_item.follow_of_dot(cfg);
        for (auto const &body : cfg.get_bodies(head)) {
          LR_1_item item(LR_0_item{head, body}, follow_set);
          nfa.add_epsilon_transition(cur_state, {item_to_nfa_state(item)});
        }
      }
      auto next_item = cur_item;
      next_item.go();
      nfa.add_transition({cur_state, symbol}, {item_to_nfa_state(next_item)});
    }

    auto [dfa, dfa_to_nfa_state_map] = nfa.to_DFA_with_mapping();
    for (auto const &[nfa_state_set, dfa_state] : dfa_to_nfa_state_map) {
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

  const LR_1_item_set &DK_1_DFA::get_LR_1_item_set(state_type state) const {
    assert(collection.contains(state));
    auto it = collection.find(state);
    if (it == collection.end()) {
      throw std::runtime_error("invalid state");
    }
    return it->second;
  }

  DK_1_DFA::state_type DK_1_DFA::get_reject_state() const {
    std::optional<state_type> reject_state;
    for (auto const &[state, item_set] : collection) {
      if (item_set.empty()) {
        reject_state = state;
        break;
      }
    }
    return reject_state.value();
  }

  std::string DK_1_DFA::MMA_draw(const CFG &cfg) const {
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
