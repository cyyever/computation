/*!
 * \file nfa.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include "nfa.hpp"

#include <sstream>
#include <vector>

#include <cyy/algorithm/graph/graph.hpp>

namespace cyy::computation {

  NFA::state_set_type NFA::go(const state_set_type &T,
                              input_symbol_type a) const {
    state_set_type direct_reachable;

    for (const auto &s : T) {
      auto it = transition_function.find({s, a});
      if (it != transition_function.end()) {
        direct_reachable.insert_range(state_set_type(it->second));
      }
    }

    state_set_type res;
    for (auto const &d : direct_reachable) {
      auto const &closure = get_epsilon_closure(d);
      res.insert_range(state_set_type(closure));
    }
    return res;
  }
  bool NFA::recognize(symbol_string_view view) const {
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
      for (auto a : alphabet->get_view()) {
        auto res = go(subset, a);
        auto [it, has_emplaced] = nfa_and_dfa_states.insert({res, next_state});
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
    if (epsilon_closure_refresh.contains(s)) {
      return epsilon_closures[s];
    }

    if (!epsilon_transition_function.contains(s)) {
      epsilon_closure_refresh.insert(s);
      epsilon_closures[s].insert(s);
      return epsilon_closures[s];
    }

    cyy::algorithm::directed_graph<state_type> epsilon_graph;
    for (const auto &[from_state, to_state_set] : epsilon_transition_function) {
      for (auto to_state : to_state_set) {
        if (from_state != to_state) {
          epsilon_graph.add_edge({from_state, to_state});
        }
      }
    }
    const auto s_index = epsilon_graph.get_vertex_index(s);
    bool has_cycle = false;
    state_set_type connect_component;
    epsilon_graph.recursive_depth_first_search(
        s_index,
        [this, &epsilon_graph, &has_cycle, &connect_component](auto u, auto v) {
          auto u_vertex = epsilon_graph.get_vertex(u);
          connect_component.insert(u_vertex);
          auto v_vertex = epsilon_graph.get_vertex(v);
          if (connect_component.contains(v_vertex)) {
            has_cycle = true;
          } else {
            connect_component.insert(v_vertex);
          }
          auto &closure = epsilon_closures[u_vertex];
          auto &v_closure = epsilon_closures[v_vertex];
          closure.insert(u_vertex);
          v_closure.insert(v_vertex);
          closure.insert_range(state_set_type(epsilon_closures[v_vertex]));
          return false;
        });
    if (!has_cycle) {
      epsilon_closure_refresh.insert_range(connect_component);
    }
    epsilon_closure_refresh.insert(s);

    return epsilon_closures[s];
  }

} // namespace cyy::computation
