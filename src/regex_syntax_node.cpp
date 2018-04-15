/*!
 * \file regex.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "regex.hpp"

namespace cyy::lang {

NFA regex::basic_node::to_NFA(const ALPHABET &alphabet,
                              uint64_t start_state) const {
  return {{start_state, start_state + 1},
          alphabet.name(),
          start_state,
          {{{start_state, symbol}, {start_state + 1}}},
          {start_state + 1}};
}

void regex::basic_node::assign_position(
    std::map<uint64_t, symbol_type> &position_to_symbol) {
  if (position_to_symbol.empty()) {
    position = 1;
  } else {
    position = position_to_symbol.rbegin()->first + 1;
  }
  position_to_symbol.insert({position, symbol});
}

std::set<uint64_t> regex::basic_node::first_pos() const { return {position}; }
std::set<uint64_t> regex::basic_node::last_pos() const { return {position}; }

NFA regex::epsilon_node::to_NFA(const ALPHABET &alphabet,
                                uint64_t start_state) const {
  return {{start_state, start_state + 1},
          alphabet.name(),
          start_state,
          {{{start_state, alphabet.get_epsilon()}, {start_state + 1}}},
          {start_state + 1}};
}

void regex::epsilon_node::assign_position(
    std::map<uint64_t, symbol_type> &position_to_symbol[[maybe_unused]]) {}

std::set<uint64_t> regex::epsilon_node::first_pos() const { return {}; }
std::set<uint64_t> regex::epsilon_node::last_pos() const { return {}; }

NFA regex::union_node::to_NFA(const ALPHABET &alphabet,
                              uint64_t start_state) const {
  auto left_NFA = left_node->to_NFA(alphabet, start_state + 1);
  auto &left_final_states = left_NFA.get_final_states();

  auto right_NFA =
      right_node->to_NFA(alphabet, *(left_final_states.begin()) + 1);
  auto &right_final_states = right_NFA.get_final_states();
  auto final_state = (*right_final_states.begin()) + 1;

  NFA nfa({start_state,final_state},alphabet.name(),start_state,{},{});
  nfa.add_sub_NFA(left_NFA,true);
  nfa.add_sub_NFA(right_NFA,true);

  for(auto const &s:nfa.get_final_states()) {
    nfa.get_transition_table()[{s, alphabet.get_epsilon()}] = {
        final_state};
  }
  nfa.replace_final_states({final_state});
  return nfa;
}

void regex::union_node::assign_position(
    std::map<uint64_t, symbol_type> &position_to_symbol) {
  left_node->assign_position(position_to_symbol);
  right_node->assign_position(position_to_symbol);
}

std::set<uint64_t> regex::union_node::first_pos() const {
  auto tmp = left_node->first_pos();
  tmp.merge(right_node->first_pos());
  return tmp;
}
std::set<uint64_t> regex::union_node::last_pos() const {
  auto tmp = left_node->last_pos();
  tmp.merge(right_node->last_pos());
  return tmp;
}
std::map<uint64_t, std::set<uint64_t>> regex::union_node::follow_pos() const {
  auto res = left_node->follow_pos();
  res.merge(right_node->follow_pos());
  return res;
}

NFA regex::concat_node::to_NFA(const ALPHABET &alphabet,
                               uint64_t start_state) const {

  auto left_NFA = left_node->to_NFA(alphabet, start_state);
  const auto &left_final_states = left_NFA.get_final_states();

  auto right_NFA_start_state= *(left_final_states.begin());
  auto right_NFA = right_node->to_NFA(alphabet, right_NFA_start_state);
  auto right_final_states =right_NFA.get_final_states();
  left_NFA.add_sub_NFA(right_NFA,false);
  left_NFA.replace_final_states(right_final_states);

  return left_NFA;
}

void regex::concat_node::assign_position(
    std::map<uint64_t, symbol_type> &position_to_symbol) {
  left_node->assign_position(position_to_symbol);
  right_node->assign_position(position_to_symbol);
}

std::set<uint64_t> regex::concat_node::first_pos() const {
  if (!left_node->nullable()) {
    return left_node->first_pos();
  }
  auto tmp = left_node->first_pos();
  tmp.merge(right_node->first_pos());
  return tmp;
}

std::set<uint64_t> regex::concat_node::last_pos() const {
  if (!right_node->nullable()) {
    return right_node->last_pos();
  }
  auto tmp = left_node->last_pos();
  tmp.merge(right_node->last_pos());
  return tmp;
}

std::map<uint64_t, std::set<uint64_t>> regex::concat_node::follow_pos() const {
  auto res = left_node->follow_pos();
  res.merge(right_node->follow_pos());

  auto tmp = right_node->first_pos();
  if (tmp.empty()) {
    return res;
  }
  for (auto pos : left_node->last_pos()) {
    res[pos].insert(tmp.begin(), tmp.end());
  }
  return res;
}

NFA regex::kleene_closure_node::to_NFA(const ALPHABET &alphabet,
                                       uint64_t start_state) const {
  auto inner_start_state = start_state + 1;
  auto inner_NFA = inner_node->to_NFA(alphabet, inner_start_state);
  auto inner_final_states = inner_NFA.get_final_states();
  auto final_state = (*inner_final_states.begin()) + 1;

  NFA nfa({start_state,final_state},alphabet.name(),start_state,{},{});
  nfa.add_sub_NFA(inner_NFA,true);

  nfa.get_transition_table()[{start_state, alphabet.get_epsilon()}].insert(
       final_state);

  for (auto const &inner_final_state : inner_final_states) {
    nfa.get_transition_table()[{inner_final_state, alphabet.get_epsilon()}] = {
        inner_start_state, final_state};
  }

  nfa.replace_final_states({final_state});
  return nfa;
}

void regex::kleene_closure_node::assign_position(
    std::map<uint64_t, symbol_type> &position_to_symbol) {
  inner_node->assign_position(position_to_symbol);
}

std::set<uint64_t> regex::kleene_closure_node::first_pos() const {
  return inner_node->first_pos();
}
std::set<uint64_t> regex::kleene_closure_node::last_pos() const {
  return inner_node->last_pos();
}

std::map<uint64_t, std::set<uint64_t>>
regex::kleene_closure_node::follow_pos() const {
  auto res = inner_node->follow_pos();
  auto tmp = inner_node->first_pos();
  if (tmp.empty()) {
    return res;
  }
  for (auto pos : inner_node->last_pos()) {
    res[pos].insert(tmp.begin(), tmp.end());
  }
  return res;
}
} // namespace cyy::lang
