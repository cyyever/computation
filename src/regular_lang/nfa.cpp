/*!
 * \file nfa.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <cassert>
#include <chrono>
#include <vector>

#include "nfa.hpp"
#include "../util.hpp"

namespace cyy::computation {

  std::set<NFA::state_type> NFA::move(const std::set<state_type> &T,
                                      symbol_type a) const {
    std::set<state_type> direct_reachable;

    for (const auto &s : T) {
      auto it = transition_table.find({a, s});
      if (it != transition_table.end()) {
        direct_reachable.insert(it->second.begin(), it->second.end());
      }
    }

    std::set<state_type> res;
    for (auto const &d : direct_reachable) {
      auto const &closure = epsilon_closure(d);
      res.insert(closure.begin(), closure.end());
    }
    return res;
  }

  const std::set<NFA::state_type> &NFA::epsilon_closure(state_type s) const {
    auto it = epsilon_closures.find(s);
    if (it != epsilon_closures.end()) {
      return it->second;
    }
    auto t1 = std::chrono::steady_clock::now();

    std::map<state_type, std::vector<state_type>> dependency;
    std::set<state_type> unstable_states{s};
    std::vector<state_type> stack{s};
    auto epsilon = alphabet->get_epsilon();
    for (size_t i = 0; i < stack.size(); i++) {
      auto unstable_state = stack[i];
      auto it2 = transition_table.find({epsilon, unstable_state});
      if (it2 == transition_table.end()) {
        continue;
      }

      for (auto next_state : it2->second) {
        auto it3 = epsilon_closures.find(next_state);
        if (it3 != epsilon_closures.end()) {
          epsilon_closures[unstable_state].insert(it3->second.begin(),
                                                  it3->second.end());
        } else {
          if (unstable_states.insert(next_state).second) {
            stack.push_back(next_state);
          }
          dependency[next_state].push_back(unstable_state);
        }
      }
    }

    for (auto unstable_state : unstable_states) {
      epsilon_closures[unstable_state].insert(unstable_state);
    }


    auto t3 = std::chrono::steady_clock::now();
    auto [sorted_states,remain_dependency]=topological_sort(dependency);
    auto t4 = std::chrono::steady_clock::now();

    std::cout << "sort took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3)
                     .count()
              << "us.\n";



    for(auto sorted_state:sorted_states) {
      for (auto prev_state : dependency[sorted_state]) {
        std::set<state_type> diff;
        auto &prev_epsilon_closure = epsilon_closures[prev_state];
        auto &unstable_epsilon_closure = epsilon_closures[sorted_state];
        std::set_difference(
            unstable_epsilon_closure.begin(), unstable_epsilon_closure.end(),
            prev_epsilon_closure.begin(), prev_epsilon_closure.end(),
            std::inserter(diff, diff.begin()));

        if (!diff.empty()) {
          prev_epsilon_closure.merge(diff);
        }
      }
      unstable_states.erase(sorted_state);
    }


    while (!unstable_states.empty()) {
      auto it = unstable_states.begin();
      auto unstable_state = *it;
      unstable_states.erase(it);
      for (auto prev_state : remain_dependency[unstable_state]) {
        std::set<state_type> diff;
        auto &prev_epsilon_closure = epsilon_closures[prev_state];
        auto &unstable_epsilon_closure = epsilon_closures[unstable_state];
        std::set_difference(
            unstable_epsilon_closure.begin(), unstable_epsilon_closure.end(),
            prev_epsilon_closure.begin(), prev_epsilon_closure.end(),
            std::inserter(diff, diff.begin()));

        if (!diff.empty()) {
          prev_epsilon_closure.merge(diff);
          unstable_states.insert(prev_state);
        }
      }
    }
    auto t2 = std::chrono::steady_clock::now();
    std::cout << "Printing took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)
                     .count()
              << "us.\n";
    return epsilon_closures[s];
  }

  std::pair<DFA, std::unordered_map<DFA::state_type, std::set<NFA::state_type>>>
  NFA::to_DFA_with_mapping() const {
    std::map<std::set<state_type>, state_type> subsets{
        {epsilon_closure(start_state), 0}};
    state_type next_state = 1;

    DFA::transition_table_type DFA_transition_table;
    for (auto it = subsets.begin(); it != subsets.end(); it++) {
      auto t1 = std::chrono::steady_clock::now();
      alphabet->foreach_symbol([&](auto const &a) {
        auto res = move(it->first, a);
        if(res.empty()) {
        std::cout<<"empty res"<<std::endl;
        }

        auto [it2, has_emplaced] =
            subsets.try_emplace(std::move(res), next_state);
        if (has_emplaced) {
          next_state++;
        }
        DFA_transition_table[{a, it->second}] = it2->second;
      });
      auto t2 = std::chrono::steady_clock::now();
      std::cout << "move subset took "
                << std::chrono::duration_cast<std::chrono::milliseconds>(t2 -
                                                                         t1)
                       .count()
                << "us.\n";
    }

    std::set<state_type> DFA_states;
    std::set<state_type> DFA_final_states;
    std::unordered_map<state_type, std::set<state_type>> state_map;
    for (auto const &[subset, DFA_state] : subsets) {
      DFA_states.insert(DFA_state);
      if (contain_final_state(subset)) {
        DFA_final_states.insert(DFA_state);
      }
      state_map.emplace(DFA_state, subset);
    }

    return {{DFA_states, alphabet->get_name(), 0, DFA_transition_table,
             DFA_final_states},
            state_map};
  }

  DFA NFA::to_DFA() const { return to_DFA_with_mapping().first; }
} // namespace cyy::computation
