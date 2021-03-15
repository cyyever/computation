/*!
 * \file nfa.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include "nfa.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>
#include <vector>

#include "../util.hpp"

namespace cyy::computation {

  NFA::state_set_type NFA::go(const state_set_type &T, symbol_type a) const {
    state_set_type direct_reachable;

    for (const auto &s : T) {
      auto it = transition_function.find({s, a});
      if (it != transition_function.end()) {
        direct_reachable.insert(it->second.begin(), it->second.end());
      }
    }

    state_set_type res;
    for (auto const &d : direct_reachable) {
      auto const &closure = get_epsilon_closure(d);
      res.insert(closure.begin(), closure.end());
    }
    return res;
  }
  bool NFA::recognize(symbol_string_view view) const {
    epsilon_closures.clear();
    auto s = get_epsilon_closure(get_start_state());
    for (auto const &symbol : view) {
      s = go(s, symbol);
      if (s.empty()) {
        return false;
      }
    }
    return contain_final_state(s);
  }

  std::pair<DFA, boost::bimap<NFA::state_set_type, DFA::state_type>>
  NFA::to_DFA_with_mapping() const {
    DFA::transition_function_type DFA_transition_function;
    boost::bimap<state_set_type, state_type> nfa_and_dfa_states;
    nfa_and_dfa_states.insert({get_epsilon_closure(get_start_state()), 0});

    state_type next_state = 1;
    std::vector iteraters{nfa_and_dfa_states.begin()};
    for (state_type dfa_state = 0; dfa_state < next_state; dfa_state++) {
      auto const &[subset, state] = *iteraters[dfa_state];
      for (auto a : *alphabet) {
        auto res = go(subset, a);
        auto [it, has_emplaced] =
            nfa_and_dfa_states.insert({std::move(res), next_state});
        if (has_emplaced) {
          iteraters.emplace_back(it);
          next_state++;
        }
        DFA_transition_function[{dfa_state, a}] = it->right;
      }
    }

    state_set_type DFA_states;
    state_set_type DFA_final_states;
    for (auto const &[subset, DFA_state] : nfa_and_dfa_states) {
      DFA_states.insert(DFA_state);
      if (contain_final_state(subset)) {
        DFA_final_states.insert(DFA_state);
      }
    }

    return {
        {DFA_states, alphabet, 0, DFA_transition_function, DFA_final_states},
        nfa_and_dfa_states};
  }

  DFA NFA::to_DFA() const { return to_DFA_with_mapping().first; }

  std::string NFA::MMA_draw() const {
    std::stringstream is;
    is << "Graph[{";
    for (auto const &[situation, next_state_set] : transition_function) {
      for (auto my_next_state : next_state_set) {
        is << "Labeled[ " << situation.state << "->" << my_next_state << ","
           << alphabet->MMA_draw(situation.input_symbol) << "],";
      }
    }
    for (auto const &[from_state, next_state_set] :
         epsilon_transition_function) {
      for (auto my_next_state : next_state_set) {
        is << "Style[Labeled[ " << from_state << "->" << my_next_state
           << ",\\[Epsilon]],Dashed],";
      }
    }
    // drop last ,
    is.seekp(-1, std::ios_base::end);
    is << "}," << finite_automaton::MMA_draw() << ']';
    return is.str();
  }

  const NFA::state_set_type &NFA::get_epsilon_closure(state_type s) const {
    auto it = epsilon_closures.find(s);
    if (it != epsilon_closures.end()) {
      return it->second;
    }

    std::unordered_map<state_type, state_set_type> dependency;
    state_set_type unstable_states{s};
    std::vector<state_type> stack{s};
    for (size_t i = 0; i < stack.size(); i++) {
      auto unstable_state = stack[i];
      auto it2 = epsilon_transition_function.find(unstable_state);
      if (it2 == epsilon_transition_function.end()) {
        continue;
      }

      for (auto next_state : it2->second) {
        auto it3 = epsilon_closures.find(next_state);
        if (it3 != epsilon_closures.end()) {
          epsilon_closures[unstable_state].merge(state_set_type(it3->second));
        } else {
          if (unstable_states.insert(next_state).second) {
            stack.push_back(next_state);
          }
          dependency[next_state].insert(unstable_state);
        }
      }
    }

    for (auto unstable_state : unstable_states) {
      epsilon_closures[unstable_state].insert(unstable_state);
    }

    auto [sorted_states, remain_dependency] = topological_sort(dependency);

    for (auto sorted_state : sorted_states) {
      for (auto prev_state : dependency[sorted_state]) {
        state_set_type diff;
        auto &prev_epsilon_closure = epsilon_closures[prev_state];
        auto &unstable_epsilon_closure = epsilon_closures[sorted_state];
        std::ranges::set_difference(unstable_epsilon_closure,
                                    prev_epsilon_closure,
                                    std ::inserter(diff, diff.begin()));

        if (!diff.empty()) {
          prev_epsilon_closure.merge(std::move(diff));
        }
      }
      unstable_states.erase(sorted_state);
    }

    while (!unstable_states.empty()) {
      auto it2 = unstable_states.begin();
      auto unstable_state = *it2;
      unstable_states.erase(it2);
      for (auto prev_state : remain_dependency[unstable_state]) {
        state_set_type diff;
        auto &prev_epsilon_closure = epsilon_closures[prev_state];
        auto &unstable_epsilon_closure = epsilon_closures[unstable_state];
        std::ranges::set_difference(unstable_epsilon_closure,
                                    prev_epsilon_closure,
                                    std::inserter(diff, diff.begin()));

        if (!diff.empty()) {
          prev_epsilon_closure.merge(std::move(diff));
          unstable_states.insert(prev_state);
        }
      }
    }
    return epsilon_closures[s];
  }

} // namespace cyy::computation
