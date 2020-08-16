/*!
 * \file dfa.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <vector>

#include "../exception.hpp"
#include "dfa.hpp"

namespace cyy::computation {
  bool DFA::equivalent_with(const DFA &rhs) const {

    if (alphabet != rhs.alphabet) {
      return false;
    }
    if (get_state_set().size() != rhs.get_state_set().size()) {
      return false;
    }
    if (final_states.size() != rhs.final_states.size()) {
      return false;
    }

    if (transition_function.size() != rhs.transition_function.size()) {
      return false;
    }

    std::unordered_map<state_type, state_type> state_map{
        {get_start_state(), rhs.get_start_state()}};
    while (true) {

      bool new_mapping = false;

      for (auto const &[situation, my_next_state] : transition_function) {
        auto it = state_map.find(situation.state);
        if (it == state_map.end()) {
          continue;
        }

        auto it2 =
            rhs.transition_function.find({it->second, situation.input_symbol});
        if (it2 == rhs.transition_function.end()) {
          return false;
        }
        auto rhs_next_state = it2->second;

        auto [it3, has_insert] =
            state_map.insert({my_next_state, rhs_next_state});
        if (has_insert) {
          new_mapping = true;
        } else {
          if (it3->second != rhs_next_state) {
            return false;
          }
        }
      }

      if (!new_mapping) {
        break;
      }
    }
    if (state_map.size() != get_state_set().size()) {
      return false;
    }

    return std::ranges::all_of(
        final_states, [&rhs](auto s) { return rhs.final_states.contains(s); });
  }

  DFA DFA::minimize() const {
    state_set_type non_final_states;
    std::ranges::set_difference(
        get_state_set(), final_states,
        std::insert_iterator(non_final_states, non_final_states.begin()));

    std::vector<state_set_type> groups{non_final_states, final_states};
    std::unordered_map<state_type, size_t> state_location;

    while (true) {
      for (size_t i = 0; i < groups.size(); i++) {
        for (auto state : groups[i]) {
          state_location[state] = i;
        }
      }

      bool has_new_group = false;
      decltype(groups) new_groups;
      for (auto const &group : groups) {
        if (group.size() <= 1) {
          new_groups.push_back(group);
          continue;
        }

        decltype(groups) sub_groups;

        auto it = group.begin();
        sub_groups.push_back({*it});
        it++;
        for (; it != group.end(); it++) {
          auto state = *it;
          bool in_new_group = true;
          for (auto &sub_group : sub_groups) {
            bool in_group =
                std::ranges::all_of(alphabet->get_view(), [&](auto const a) {
                  return state_location[go(*(sub_group.begin()), a).value()] ==
                         state_location[go(state, a).value()];
                });
            if (in_group) {
              sub_group.insert(state);
              in_new_group = false;
              break;
            }
          }
          if (in_new_group) {
            has_new_group = true;
            sub_groups.push_back({state});
          }
        }
        new_groups.insert(new_groups.end(),
                          std::move_iterator(sub_groups.begin()),
                          std::move_iterator(sub_groups.end()));
      }
      if (!has_new_group) {
        break;
      }
      groups = std::move(new_groups);
    }
    state_type minimize_DFA_start_state{};
    state_set_type minimize_DFA_states;
    state_set_type minimize_DFA_final_states;
    transition_function_type minimize_DFA_transition_function;
    for (size_t i = 0; i < groups.size(); i++) {
      minimize_DFA_states.insert(i);
      if (groups[i].contains(this->get_start_state())) {
        minimize_DFA_start_state = i;
      }

      for (auto const &state : groups[i]) {
        if (final_states.contains(state)) {
          minimize_DFA_final_states.insert(i);
          break;
        }
      }

      for (auto a : *alphabet) {
        auto next_state = go(*(groups[i].begin()), a).value();
        minimize_DFA_transition_function[{i, a}] = state_location[next_state];
      }
    }
    return {minimize_DFA_states, alphabet->get_name(), minimize_DFA_start_state,
            minimize_DFA_transition_function, minimize_DFA_final_states};
  }
  void DFA::mark_live_states() const {
    if (live_states_opt.has_value()) {
      return;
    }
    std::unordered_map<state_type, state_set_type> state_dep;
    for (auto const &[p, next_state] : transition_function) {
      state_dep[next_state].insert(p.state);
    }
    auto live_states = final_states;
    while (true) {
      auto live_state_size = live_states.size();
      state_set_type new_live_states;
      for (auto s : live_states) {
        new_live_states.merge(state_set_type(state_dep[s]));
      }
      live_states.merge(std::move(new_live_states));
      if (live_states.size() == live_state_size) {
        break;
      }
    }
    live_states_opt = live_states;
  }

  DFA DFA::intersect(const DFA &rhs) const {
    if (alphabet != rhs.alphabet) {
      throw exception::unmatched_alphabets(alphabet->get_name() + " and " +
                                           rhs.alphabet->get_name());
    }
    std::unordered_map<std::pair<state_type, state_type>, state_type>
        state_products;
    state_set_type result_states;
    state_set_type result_final_states;
    state_type result_start_state{};
    transition_function_type result_transition_function;
    state_type next_state = 0;
    for (auto s1 : get_state_set()) {
      for (auto s2 : rhs.get_state_set()) {
        state_products.try_emplace({s1, s2}, next_state);
        result_states.insert(next_state);
        if (s1 == get_start_state() && s2 == rhs.get_start_state()) {
          result_start_state = next_state;
        }
        if (is_final_state(s1) && rhs.is_final_state(s2)) {
          result_final_states.insert(next_state);
        }

        next_state++;
      }
    }

    for (auto const &[product, result_state] : state_products) {
      for (auto a : *alphabet) {
        auto it = state_products.find(
            {go(product.first, a).value(), rhs.go(product.second, a).value()}

        );
        result_transition_function[{result_state, a}] = it->second;
      }
    }
    return {result_states, alphabet->get_name(), result_start_state,
            result_transition_function, result_final_states};
  }

  DFA DFA::complement() const {
    state_set_type new_final_states;
    std::ranges::set_difference(
        get_state_set(), final_states,
        std::inserter(new_final_states, new_final_states.begin()));
    return {get_state_set(), alphabet->get_name(), get_start_state(),
            transition_function, new_final_states};
  }
  std::string DFA::MMA_draw() const {
    std::stringstream is;
    is << "Graph[{";
    for (auto const &[situation, my_next_state] : transition_function) {
      is << "Labeled[ " << situation.state << "->" << my_next_state << ",\""
         << alphabet->to_string(situation.input_symbol) << "\"],";
    }
    // drop last ,
    is.seekp(-1, std::ios_base::end);
    is << "}," << finite_automaton::MMA_draw() << ']';
    return is.str();
  }
} // namespace cyy::computation
