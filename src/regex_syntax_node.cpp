/*!
 * \file regex.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "regex.hpp"

namespace cyy::lang {

  NFA regex::basic_node::to_NFA(const ALPHABET &alphabet,uint64_t start_state) const {
    return {{start_state,start_state+1}, alphabet.name(),start_state,{{  {start_state,symbol}   ,{start_state+1}  }},{start_state+1}};
  }

  void regex::basic_node::assign_position( std::map<uint64_t,symbol_type> &position_to_symbol) {
    if(position_to_symbol.empty()) {
      position=0;
    } else {
      position=position_to_symbol.end()->first+1;
    }
    position_to_symbol.insert({position,symbol});
    return;
  }

  std::set<uint64_t> regex::basic_node::first_pos() const {
    return {position};
  }
  std::set<uint64_t> regex::basic_node::last_pos() const {
    return {position};
  }

  NFA regex::epsilon_node::to_NFA(const ALPHABET &alphabet,uint64_t start_state) const {
    return {{start_state,start_state+1}, alphabet.name(),start_state,{{  {start_state,alphabet.get_epsilon()}   ,{start_state+1}  }},{start_state+1}};
  }

  void regex::epsilon_node::assign_position( std::map<uint64_t,symbol_type> &position_to_symbol) {
    return;
  }

  std::set<uint64_t> regex::epsilon_node::first_pos() const {
    return {};
  }
  std::set<uint64_t> regex::epsilon_node::last_pos() const {
    return {};
  }

  NFA regex::union_node::to_NFA(const ALPHABET &alphabet,uint64_t start_state) const {
    auto left_NFA=left_node->to_NFA(alphabet,start_state+1);
    auto left_states=left_NFA.get_states();
    auto left_final_states=left_NFA.get_final_states();
    auto left_start_state=left_NFA.get_start_state();
    auto left_transition_table=left_NFA.get_transition_table();

    auto right_NFA=right_node->to_NFA(alphabet,start_state+1);
    auto right_states=right_NFA.get_states();
    auto right_final_states=right_NFA.get_final_states();
    auto right_start_state=right_NFA.get_start_state();
    auto right_transition_table=right_NFA.get_transition_table();

    left_states.merge(right_states);
    left_transition_table.merge(right_transition_table);
    
    left_states.insert(start_state);
    auto final_state= start_state+left_states.size()+1;
    left_states.insert(final_state);

    left_transition_table[{start_state,alphabet.get_epsilon()}]={left_start_state,right_start_state};
    for(auto const &left_final_state:left_final_states) {
      left_transition_table[{left_final_state,alphabet.get_epsilon()}]={final_state};
    }
    for(auto const &right_final_state:right_final_states) {
      left_transition_table[{right_final_state,alphabet.get_epsilon()}]={final_state};
    }

    return {left_states, alphabet.name(),start_state,left_transition_table,{final_state}};
  }

  void regex::union_node::assign_position( std::map<uint64_t,symbol_type> &position_to_symbol) {
    left_node->assign_position(position_to_symbol);
    right_node->assign_position(position_to_symbol);
    return;
  }

  std::set<uint64_t> regex::union_node::first_pos() const {
    auto tmp=left_node->first_pos();
    tmp.merge(right_node->first_pos());
    return tmp;
  }
  std::set<uint64_t> regex::union_node::last_pos() const {
    auto tmp=left_node->last_pos();
    tmp.merge(right_node->last_pos());
    return tmp;
  }
  std::map<uint64_t,std::set<uint64_t>> regex::union_node::follow_pos()const {
    auto tmp=left_node->follow_pos();
    tmp.merge(right_node->follow_pos());
    return tmp;
  }   

  NFA regex::concat_node::to_NFA(const ALPHABET &alphabet,uint64_t start_state) const {
    auto left_NFA=left_node->to_NFA(alphabet,start_state);
    auto left_states=left_NFA.get_states();
    auto left_final_states=left_NFA.get_final_states();
    auto left_transition_table=left_NFA.get_transition_table();

    auto right_NFA=right_node->to_NFA(alphabet,*(left_final_states.begin()));
    auto right_states=right_NFA.get_states();
    auto right_transition_table=right_NFA.get_transition_table();

    left_states.merge(right_states);
    left_transition_table.merge(right_transition_table);
    
    return {left_states, alphabet.name(),start_state,left_transition_table,right_NFA.get_final_states()};
}

  void regex::concat_node::assign_position( std::map<uint64_t,symbol_type> &position_to_symbol) {
    left_node->assign_position(position_to_symbol);
    right_node->assign_position(position_to_symbol);
    return;
  }

  std::set<uint64_t> regex::concat_node::first_pos() const {
    if(!left_node->nullable()) {
      return left_node->first_pos();
    }
    auto tmp=left_node->first_pos();
    tmp.merge(right_node->first_pos());
    return tmp;
  }

  std::set<uint64_t> regex::concat_node::last_pos() const {
    if(!right_node->nullable()) {
      return right_node->last_pos();
    }
    auto tmp=left_node->last_pos();
    tmp.merge(right_node->last_pos());
    return tmp;
  }

  std::map<uint64_t,std::set<uint64_t>> regex::concat_node::follow_pos()const {
    auto tmp=right_node->first_pos();
    if(tmp.empty()) {
      return {};
    }
    std::map<uint64_t,std::set<uint64_t>> res;
    for(auto pos:left_node->last_pos()) {
      res.insert({pos,tmp});
    }
    return res;
  }   


NFA regex::kleene_closure_node::to_NFA( const ALPHABET &alphabet,uint64_t start_state ) const {
    auto inner_start_state=start_state+1;
    auto inner_NFA=inner_node->to_NFA(alphabet,inner_start_state);
    auto inner_states=inner_NFA.get_states();
    auto inner_final_states=inner_NFA.get_final_states();
    auto inner_transition_table=inner_NFA.get_transition_table();
    inner_states.insert(start_state);
    auto final_state= inner_start_state+inner_states.size();
    inner_states.insert(final_state);

    inner_transition_table[{    start_state ,alphabet.get_epsilon()  }]={inner_start_state,final_state};
    for(auto const & inner_final_state:inner_final_states) {
      inner_transition_table[{    inner_final_state,alphabet.get_epsilon()  }]={inner_start_state,final_state};
    }

    return {inner_states, alphabet.name(),start_state,inner_transition_table,{final_state}};
}

  void regex::kleene_closure_node::assign_position( std::map<uint64_t,symbol_type> &position_to_symbol) {
    inner_node->assign_position(position_to_symbol);
    return;
  }

  std::set<uint64_t> regex::kleene_closure_node::first_pos() const {
    return inner_node->first_pos();
  }
  std::set<uint64_t> regex::kleene_closure_node::last_pos() const {
    return inner_node->last_pos();
  }

  std::map<uint64_t,std::set<uint64_t>> regex::kleene_closure_node::follow_pos()const {
    auto tmp=inner_node->first_pos();
    if(tmp.empty()) {
      return {};
    }
    std::map<uint64_t,std::set<uint64_t>> res;
    for(auto pos:inner_node->last_pos()) {
      res.insert({pos,tmp});
    }
    return res;
  }   
} // namespace cyy::lang
