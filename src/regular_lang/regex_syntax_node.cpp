/*!
 * \file regex.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "regex.hpp"

namespace cyy::computation {

  NFA regex::basic_node::to_NFA(std::string_view alphabet_name,
                                NFA::state_type start_state) const {
    return {{start_state, start_state + 1},
            alphabet_name,
            start_state,
            {{{symbol, start_state}, {start_state + 1}}},
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

  NFA regex::epsilon_node::to_NFA(std::string_view alphabet_name,
                                  NFA::state_type start_state) const {
    return {{start_state, start_state + 1},
            alphabet_name,
            start_state,
            {},
            {start_state + 1},
            {{start_state, {start_state + 1}}}};
  }

  void regex::epsilon_node::assign_position(
      std::map<uint64_t, symbol_type> &position_to_symbol
      [[maybe_unused]]) noexcept {}

  std::set<uint64_t> regex::epsilon_node::first_pos() const { return {}; }
  std::set<uint64_t> regex::epsilon_node::last_pos() const { return {}; }

  NFA regex::empty_set_node::to_NFA(std::string_view, NFA::state_type) const {
    throw std::logic_error("unsupported");
  }

  void
  regex::empty_set_node::assign_position(std::map<uint64_t, symbol_type> &) {
    throw std::logic_error("unsupported");
  }

  std::set<uint64_t> regex::empty_set_node::first_pos() const { return {}; }
  std::set<uint64_t> regex::empty_set_node::last_pos() const { return {}; }

  bool regex::union_node::is_empty_set_node() const {
    return left_node->is_empty_set_node() && right_node->is_empty_set_node();
  }

  bool regex::union_node::is_epsilon_node() const {
    if (left_node->is_epsilon_node() &&
        (right_node->is_epsilon_node() || right_node->is_empty_set_node())) {
      return true;
    }
    if (right_node->is_epsilon_node() &&
        (left_node->is_epsilon_node() || left_node->is_empty_set_node())) {
      return true;
    }
    return false;
  }

  NFA regex::union_node::to_NFA(std::string_view alphabet_name,
                                NFA::state_type start_state) const {
    auto left_NFA = left_node->to_NFA(alphabet_name, start_state + 1);
    auto left_final_states = left_NFA.get_final_states();
    auto const &left_start_state = left_NFA.get_start_state();

    auto right_NFA =
        right_node->to_NFA(alphabet_name, *(left_final_states.begin()) + 1);
    auto const &right_final_states = right_NFA.get_final_states();
    auto final_state = (*right_final_states.begin()) + 1;
    auto right_start_state = right_NFA.get_start_state();

    left_NFA.add_state(start_state);
    left_NFA.add_state(final_state);
    left_NFA.change_start_state(start_state);
    left_NFA.add_epsilon_transition(start_state, {left_start_state});
    left_NFA.add_sub_NFA(std::move(right_NFA));
    left_NFA.add_epsilon_transition(start_state, {right_start_state});

    for (auto const &s : left_NFA.get_final_states()) {
      left_NFA.replace_epsilon_transition(s, {final_state});
    }
    left_NFA.replace_final_states({final_state});
    return left_NFA;
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
  std::shared_ptr<regex::syntax_node> regex::union_node::simplify() const {
    auto new_right_node = right_node->simplify();
    if (!new_right_node) {
      new_right_node = right_node;
    }
    if (left_node->is_empty_set_node()) {
      return new_right_node;
    }
    auto new_left_node = left_node->simplify();
    if (!new_left_node) {
      new_left_node = left_node;
    }
    if (new_right_node->is_empty_set_node()) {
      return new_left_node;
    }
    if (new_left_node == left_node && new_right_node == right_node) {
      return {};
    }
    return std::make_shared<regex::union_node>(new_left_node, new_right_node);
  }

  NFA regex::concat_node::to_NFA(std::string_view alphabet_name,
                                 NFA::state_type start_state) const {

    auto left_NFA = left_node->to_NFA(alphabet_name, start_state);
    const auto &left_final_states = left_NFA.get_final_states();

    auto right_NFA_start_state = *(left_final_states.begin());
    auto right_NFA = right_node->to_NFA(alphabet_name, right_NFA_start_state);
    auto right_final_states = right_NFA.get_final_states();
    left_NFA.add_sub_NFA(std::move(right_NFA));
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

  std::map<uint64_t, std::set<uint64_t>>
  regex::concat_node::follow_pos() const {
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

  bool regex::concat_node::is_epsilon_node() const {
    return left_node->is_epsilon_node() && right_node->is_epsilon_node();
  }

  bool regex::concat_node::is_empty_set_node() const {
    return left_node->is_empty_set_node() || right_node->is_empty_set_node();
  }
  std::shared_ptr<regex::syntax_node> regex::concat_node::simplify() const {
    if (this->is_empty_set_node()) {
      return std::make_shared<regex::empty_set_node>();
    }
    auto new_left_node = left_node->simplify();
    if (!new_left_node) {
      new_left_node = left_node;
    }
    if (right_node->is_epsilon_node()) {
      return new_left_node;
    }
    auto new_right_node = right_node->simplify();
    if (!new_right_node) {
      new_right_node = right_node;
    }
    if (left_node->is_epsilon_node()) {
      return new_right_node;
    }
    if (new_left_node == left_node && new_right_node == right_node) {
      return {};
    }
    return std::make_shared<regex::concat_node>(new_left_node, new_right_node);
  }

  NFA regex::kleene_closure_node::to_NFA(std::string_view alphabet_name,
                                         NFA::state_type start_state) const {
    const auto inner_start_state = start_state + 1;
    auto inner_NFA = inner_node->to_NFA(alphabet_name, inner_start_state);
    auto inner_final_states = inner_NFA.get_final_states();
    auto final_state = (*inner_final_states.begin()) + 1;

    NFA nfa({start_state, final_state}, alphabet_name, start_state, {}, {});
    nfa.add_sub_NFA(std::move(inner_NFA));
    nfa.add_epsilon_transition(start_state, {inner_start_state});
    nfa.add_epsilon_transition(start_state, {final_state});

    for (auto const &inner_final_state : inner_final_states) {
      nfa.replace_epsilon_transition(inner_final_state,
                                     {inner_start_state, final_state});
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
  bool regex::kleene_closure_node::is_epsilon_node() const {
    return inner_node->is_epsilon_node() || inner_node->is_empty_set_node();
  }

  bool regex::kleene_closure_node::is_empty_set_node() const { return false; }

  std::shared_ptr<regex::syntax_node>
  regex::kleene_closure_node::simplify() const {
    if (this->is_epsilon_node()) {
      return std::make_shared<regex::epsilon_node>();
    }
    auto new_inner_node = inner_node->simplify();
    if (!new_inner_node) {
      new_inner_node = inner_node;
    }
    if (new_inner_node == inner_node) {
      return {};
    }
    return std::make_shared<regex::kleene_closure_node>(new_inner_node);
  }
} // namespace cyy::computation
