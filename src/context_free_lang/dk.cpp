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

    auto alphabet_of_nonterminals = std::make_shared<range_alphabet>(
        max_symbol + 1, max_symbol + nonterminals.size(),
        "alphabet_of_nonterminals");
    for (auto const &nonterminal : nonterminals) {
      max_symbol++;
      nonterminal_to_state.emplace(nonterminal, max_symbol);
      state_to_nonterminal.emplace(max_symbol, nonterminal);
    }
    auto nfa_alphabet = std::make_shared<union_alphabet>(
        cfg.get_alphabet_ptr(), alphabet_of_nonterminals);
    ALPHABET::set(nfa_alphabet);

    using state_type = NFA::state_type;
    using state_set_type = NFA::state_set_type;

    std::unordered_map<LR_0_item, state_type> item_to_nfa_state_map;
    std::unordered_map<state_type, LR_0_item> NFA_state_to_item_map;

    NFA nfa{{0}, nfa_alphabet->get_name(), 0, {}, {}};
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
        cyy::computation::LR_0_item init_item({head, body});
        auto state = item_to_nfa_state(init_item);
        if (head == cfg.get_start_symbol()) {
          nfa.add_epsilon_transition(nfa.get_start_state(), {state});
        }
        head_states[head].insert(state);
      }
    }

    for (auto const &[head, bodies] : cfg.get_productions()) {
      for (auto const &body : bodies) {
        cyy::computation::LR_0_item cur_item({head, body});

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
            symbol = nonterminal_to_state[grammar_symbol.get_nonterminal()];
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
    dfa_opt = std::move(dfa);
  }

  DK_DFA::DK_DFA(
      DFA dfa_,
      std::unordered_map<grammar_symbol_type::nonterminal_type, symbol_type>
          nonterminal_to_state_,
      std::unordered_map<symbol_type, grammar_symbol_type::nonterminal_type>
          state_to_nonterminal_,
      lr_0_item_set_collection_type collection_)
      : dfa_opt(std::move(dfa_)),
        nonterminal_to_state(std::move(nonterminal_to_state_)),
        state_to_nonterminal(std::move(state_to_nonterminal_)),
        collection(std::move(collection_)) {}

  DK_DFA::goto_table_type DK_DFA::get_goto_table() const {
    goto_table_type goto_table;
    for (auto const &[situation, next_state] :
         dfa_opt.value().get_transition_function()) {
      assert(collection.contains(next_state));
      if (collection.find(next_state)->second.empty()) {
        continue;
      }
      auto it = state_to_nonterminal.find(situation.input_symbol);
      if (it != state_to_nonterminal.end()) {
        goto_table[{situation.state, it->second}] = next_state;
      } else {
        goto_table[{situation.state, situation.input_symbol}] = next_state;
      }
    }
    return goto_table;
  }

  std::tuple<DFA, std::unordered_map<CFG::nonterminal_type, symbol_type>,
             std::unordered_map<symbol_type, CFG::nonterminal_type>,
             CFG::lr_0_item_set_collection_type>
  CFG::get_DK() const {
    auto max_symbol = alphabet->get_max_symbol();
    auto nonterminals = get_nonterminals();

    auto alphabet_of_nonterminals = std::make_shared<range_alphabet>(
        max_symbol + 1, max_symbol + nonterminals.size(),
        "alphabet_of_nonterminals");
    std::unordered_map<CFG::nonterminal_type, symbol_type>
        nonterminal_to_symbol;
    std::unordered_map<symbol_type, CFG::nonterminal_type>
        symbol_to_nonterminal;
    for (auto const &nonterminal : nonterminals) {
      max_symbol++;
      nonterminal_to_symbol.emplace(nonterminal, max_symbol);
      symbol_to_nonterminal.emplace(max_symbol, nonterminal);
    }
    auto nfa_alphabet =
        std::make_shared<union_alphabet>(alphabet, alphabet_of_nonterminals);
    ALPHABET::set(nfa_alphabet);

    using state_type = NFA::state_type;
    using state_set_type = NFA::state_set_type;

    std::unordered_map<LR_0_item, state_type> item_to_nfa_state_map;
    std::unordered_map<state_type, LR_0_item> NFA_state_to_item_map;

    NFA nfa{{0}, nfa_alphabet->get_name(), 0, {}, {}};
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

    std::unordered_map<nonterminal_type, state_set_type> head_states;
    for (auto const &[head, bodies] : productions) {
      for (auto const &body : bodies) {
        cyy::computation::LR_0_item init_item({head, body});
        auto state = item_to_nfa_state(init_item);
        if (head == start_symbol) {
          nfa.add_epsilon_transition(nfa.get_start_state(), {state});
        }
        head_states[head].insert(state);
      }
    }

    for (auto const &[head, bodies] : productions) {
      for (auto const &body : bodies) {
        cyy::computation::LR_0_item cur_item({head, body});

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
            symbol = nonterminal_to_symbol[grammar_symbol.get_nonterminal()];
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
    lr_0_item_set_collection_type collection;
    for (auto const &[dfa_state, nfa_state_set] : dfa_to_nfa_state_map) {
      auto &item_set = collection[dfa_state];
      for (auto const &nfa_state : nfa_state_set) {
        auto it = NFA_state_to_item_map.find(nfa_state);
        if (it != NFA_state_to_item_map.end()) {
          item_set.add_item(it->second);
        }
      }
    }
    return {dfa, nonterminal_to_symbol, symbol_to_nonterminal, collection};
  }
  const LR_0_item_set &DK_DFA::get_LR_0_item_set(state_type state) const {
    assert(collection.contains(state));
    auto it = collection.find(state);
    if (it == collection.end()) {
      throw std::runtime_error("invalid state");
    }
    return it->second;
  }

} // namespace cyy::computation
