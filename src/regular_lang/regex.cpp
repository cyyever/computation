/*!
 * \file regex.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <vector>

#include "regex.hpp"

namespace cyy::computation {

  DFA regex::to_DFA() const {
    std::map<uint64_t, symbol_type> position_to_symbol;

    auto syntax_tree_with_endmarker = std::make_shared<regex::concat_node>(
        syntax_tree,
        std::make_shared<regex::basic_node>(alphabet->get_endmarker()));

    syntax_tree_with_endmarker->assign_position(position_to_symbol);

    auto final_position = position_to_symbol.rbegin()->first;
    auto follow_pos_table = syntax_tree_with_endmarker->follow_pos();

    std::vector<bool> flags{false};
    std::vector<std::set<uint64_t>> position_sets{
        syntax_tree_with_endmarker->first_pos()};
    std::set<DFA::state_type> DFA_states{0};
    DFA::transition_table_type DFA_transition_table;
    std::set<DFA::state_type> DFA_final_states;
    for (size_t i = 0; i < flags.size(); i++) {
      if (flags[i]) {
        continue;
      }
      flags[i] = true;

      alphabet->foreach_symbol([&](auto const &a) {
        std::set<uint64_t> follow_pos_set;
        for (auto const pos : position_sets[i]) {
          if (position_to_symbol[pos] == a) {
            for (auto const follow_pos : follow_pos_table[pos]) {
              follow_pos_set.insert(follow_pos);
            }
          }
        }

        size_t j = 0;
        for (j = 0; j < flags.size(); j++) {
          if (position_sets[j] == follow_pos_set) {
            break;
          }
        }

        // new state
        if (j == flags.size()) {
          position_sets.push_back(follow_pos_set);
          flags.push_back(false);
          DFA_states.insert(j);
        }

        DFA_transition_table[{a, i}] = j;
      });
    }

    for (size_t i = 0; i < position_sets.size(); i++) {
      if (position_sets[i].count(final_position) != 0) {
        DFA_final_states.insert(i);
      }
    }

    return {DFA_states, alphabet->get_name(), 0, DFA_transition_table,
            DFA_final_states};
  }
} // namespace cyy::computation
