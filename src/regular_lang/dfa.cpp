/*!
 * \file dfa.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include "dfa.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

#include "exception.hpp"

namespace cyy::computation {
  bool DFA::equivalent_with(const DFA &rhs) const {

    if (alphabet != rhs.alphabet) {
      return false;
    }
    if (get_states().size() != rhs.get_states().size()) {
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
    if (state_map.size() != get_states().size()) {
      return false;
    }

    return std::ranges::all_of(
        final_states, [&rhs](auto s) { return rhs.final_states.contains(s); });
  }

  std::pair<DFA, std::vector<DFA::state_set_type>> DFA::minimize() const {
    state_set_type non_final_states;
    std::ranges::set_difference(
        get_states(), final_states,
        std::insert_iterator(non_final_states, non_final_states.begin()));

    std::vector<state_set_type> groups{non_final_states, final_states};
    std::unordered_map<state_type, size_t> state_to_group_index;
    bool has_new_group = true;

    while (has_new_group) {
      for (size_t i = 0; i < groups.size(); i++) {
        for (auto state : groups[i]) {
          state_to_group_index[state] = i;
        }
      }

      has_new_group = false;
      for (auto it = groups.begin(); it != groups.end(); it++) {
        auto const &group = *it;
        if (group.size() <= 1) {
          continue;
        }

        // split group on input symbol
        std::unordered_map<size_t, state_set_type> sub_groups;
        for (auto a : alphabet->get_view()) {
          sub_groups.clear();
          for (auto s : group) {
            auto next_state = go(s, a).value();
            sub_groups[state_to_group_index[next_state]].insert(s);
          }
          if (sub_groups.size() > 1) {
            has_new_group = true;
            break;
          }
          assert(sub_groups.size() == 1);
        }
        if (has_new_group) {
          bool flag = true;
          for (auto &[_, sub_group] : sub_groups) {
            if (flag) {
              *it = std::move(sub_group);
              flag = false;
            } else {
              groups.emplace_back(std::move(sub_group));
            }
          }
          break;
        }
      }
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

      for (auto a : alphabet->get_view()) {
        auto next_state = go(*(groups[i].begin()), a).value();
        minimize_DFA_transition_function[{i, a}] =
            state_to_group_index[next_state];
      }
    }
    return {DFA{std::move(minimize_DFA_states), alphabet,
                std::move(minimize_DFA_start_state),
                std::move(minimize_DFA_transition_function),
                std::move(minimize_DFA_final_states)},
            std::move(groups)};
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
    auto state_set_product = get_state_set_product(rhs.get_states());
    state_set_type result_states;
    state_set_type result_final_states;
    state_type result_start_state{};
    transition_function_type result_transition_function;
    for (auto const &[state_pair, result_state] : state_set_product) {
      auto const &[s1, s2] = state_pair;
      result_states.insert(result_state);
      if (s1 == get_start_state() && s2 == rhs.get_start_state()) {
        result_start_state = result_state;
      }
      if (is_final_state(s1) && rhs.is_final_state(s2)) {
        result_final_states.insert(result_state);
      }
      for (auto a : alphabet->get_view()) {
        auto it =
            state_set_product.find({go(s1, a).value(), rhs.go(s2, a).value()});
        result_transition_function[{result_state, a}] = it->second;
      }
    }
    return {result_states, alphabet, result_start_state,
            result_transition_function, result_final_states};
  }

  DFA DFA::complement() const {
    state_set_type new_final_states;
    std::ranges::set_difference(
        get_states(), final_states,
        std::inserter(new_final_states, new_final_states.begin()));
    return {get_states(), alphabet, get_start_state(), transition_function,
            new_final_states};
  }
  std::string DFA::MMA_draw() const {
    std::unordered_map<std::pair<state_type, state_type>, std::set<symbol_type>>
        edge_labels;
    for (auto const &[situation, my_next_state] : transition_function) {
      edge_labels[{situation.state, my_next_state}].emplace(
          situation.input_symbol);
    }
    std::stringstream is;
    is << "Graph[{";
    for (auto const &[k, v] : edge_labels) {
      auto [from_state, to_state] = k;
      is << "Labeled[ " << from_state << "->" << to_state << ",{";
      for (auto const label : v) {
        is << alphabet->MMA_draw(label) << ',';
      }
      // drop last ,
      is.seekp(-1, std::ios_base::end);
      is << "}],";
    }

    // drop last ,
    is.seekp(-1, std::ios_base::end);
    is << "}," << finite_automaton::MMA_draw() << ']';
    return is.str();
  }
} // namespace cyy::computation
