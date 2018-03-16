/*!
 * \file regex.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <vector>

#include "regex.hpp"

namespace cyy::lang {

DFA regex::to_DFA() const {
  std::map<uint64_t, symbol_type> position_to_symbol;
  syntax_tree->assign_position(position_to_symbol);
  // endmark
  auto final_position = position_to_symbol.end()->first + 1;
  auto follow_pos_table = syntax_tree->follow_pos();
  for (auto pos : syntax_tree->last_pos()) {
    follow_pos_table.insert({pos, {final_position}});
  }

  std::vector<std::set<uint64_t>> position_sets{syntax_tree->first_pos()};

  std::vector<bool> flags{false};
  std::set<uint64_t> DFA_states;
  std::map<std::pair<uint64_t, symbol_type>, uint64_t> DFA_transition_table;
  std::set<uint64_t> DFA_final_states;
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

        if (follow_pos_set.count(final_position) != 0) {
          DFA_final_states.insert(j);
        }
      }

      DFA_transition_table[{i, a}] = j;
    });
  }

  return {DFA_states, alphabet->name(), 0, DFA_transition_table,
          DFA_final_states};
}

} // namespace cyy::lang
