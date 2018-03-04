/*!
 * \file regex.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "regex.hpp"

namespace cyy::lang {

/*
 * 	regex -> regex "|" a | a
 * 	a -> a "" b | b
 * 	b -> b "*" | c
 * 	c -> (regex) | symbol
 * =>
 * 	regex -> ax
 * 	x -> "|" ax | epsilon
 * 	a -> b "" a | b
 * 	b -> cy
 * 	y -> "*" y | epsilon
 * 	c -> (regex) | symbol
 */
namespace {

std::shared_ptr<regex::syntax_node> nonterminal_regex(const ALPHABET &alphabet,
                                                      symbol_string_view &view);

std::shared_ptr<regex::syntax_node> nonterminal_c(const ALPHABET &alphabet,
                                                  symbol_string_view &view) {
  if (view.empty()) {
    return std::make_shared<regex::syntax_node>(alphabet.get_epsilon());
  }

  switch (view[0]) {
  case '(': {
    view.remove_prefix(1);
    auto sub_node = nonterminal_regex(alphabet, view);
    if (view.empty() || view[0] != ')') {
      throw std::invalid_argument("lack ')'");
    }
    view.remove_prefix(1);
    return sub_node;
    break;
  }
  case '|':
  case '*':
    throw std::invalid_argument("lack symbol");
  case '\\':
    if (view.size() < 2) {
      throw std::invalid_argument("incomplete escape sequence");
    }
    view.remove_prefix(1);
    [[fallthrough]];
  default:
    break;
  }
  if (!alphabet.contain(view[0])) {
    throw std::invalid_argument(std::string("invalid symbol ") +
                                std::to_string(view[0]));
  }
  auto sub_node = std::make_shared<regex::syntax_node>(view[0]);
  view.remove_prefix(1);
  return sub_node;
}

std::shared_ptr<regex::syntax_node>
nonterminal_y(std::shared_ptr<regex::syntax_node> && sub_node,
              symbol_string_view &view) {
  if (!view.empty() && view[0] == '*') {
    view.remove_prefix(1);
    return nonterminal_y( std::make_shared<regex::syntax_node>(sub_node) , view);
  }
  return sub_node;
}

std::shared_ptr<regex::syntax_node>
nonterminal_b(const ALPHABET &alphabet, symbol_string_view & view) {
  return nonterminal_y( nonterminal_c(alphabet,view) ,view);
}

std::shared_ptr<regex::syntax_node>
nonterminal_a(const ALPHABET &alphabet, symbol_string_view & view) {
  auto left_node= nonterminal_b(alphabet,view);
  if(!view.empty() && view[0]!='|' && view[0] !='*') {
    return std::make_shared<regex::syntax_node>( regex::syntax_node::TYPE::CONCAT, left_node, nonterminal_a(alphabet,view) );
  }
  return left_node;
}

std::shared_ptr<regex::syntax_node>
nonterminal_x( std::shared_ptr<regex::syntax_node> &&left_node, const ALPHABET &alphabet, symbol_string_view & view) {
  if(!view.empty() && view[0]=='|') {
    view.remove_prefix(1);
    return std::make_shared<regex::syntax_node>( regex::syntax_node::TYPE::UNION, left_node,
      nonterminal_x(  nonterminal_a(alphabet,view), alphabet,view));
  } else {
    return left_node;
  }
}

std::shared_ptr<regex::syntax_node>
nonterminal_regex(const ALPHABET &alphabet, symbol_string_view & view) {
  return nonterminal_x(   nonterminal_a(alphabet,view), alphabet,view);
}

} // namespace
std::shared_ptr<regex::syntax_node> regex::parse(symbol_string_view view)  const {
   return nonterminal_regex(*alphabet,view);
}

NFA regex::to_NFA(const std::shared_ptr<syntax_node> &tree,uint64_t start_state) const  {
  if(tree->type==syntax_node::TYPE::BASIC) {
    return {{start_state,start_state+1}, alphabet->name(),start_state,{{  {start_state,tree->symbol}   ,{start_state+1}  }},{start_state+1}};
  } else if(tree->type==syntax_node::TYPE::UNION) {
    auto left_NFA=to_NFA(tree->left_node,start_state+1);
    auto left_states=left_NFA.get_states();
    auto left_final_states=left_NFA.get_final_states();
    auto left_start_state=left_NFA.get_start_state();
    auto left_transition_table=left_NFA.get_transition_table();

    auto right_NFA=to_NFA(tree->right_node, start_state+left_states.size()+1);
    auto right_states=right_NFA.get_states();
    auto right_final_states=right_NFA.get_final_states();
    auto right_start_state=right_NFA.get_start_state();
    auto right_transition_table=right_NFA.get_transition_table();

    left_states.merge(right_states);
    left_transition_table.merge(right_transition_table);
    
    left_states.insert(start_state);
    auto final_state= start_state+left_states.size()+1;
    left_states.insert(final_state);

    left_transition_table[{start_state,alphabet->get_epsilon()}]={left_start_state,right_start_state};
    for(auto const &left_final_state:left_final_states) {
      left_transition_table[{left_final_state,alphabet->get_epsilon()}]={final_state};
    }
    for(auto const &right_final_state:right_final_states) {
      left_transition_table[{right_final_state,alphabet->get_epsilon()}]={final_state};
    }

    return {left_states, alphabet->name(),start_state,left_transition_table,{final_state}};

  } else if(tree->type==syntax_node::TYPE::CONCAT) {
    auto left_NFA=to_NFA(tree->left_node,start_state);
    auto left_states=left_NFA.get_states();
    auto left_final_states=left_NFA.get_final_states();
    auto left_transition_table=left_NFA.get_transition_table();

    auto right_NFA=to_NFA(tree->right_node, *(left_final_states.begin()));
    auto right_states=right_NFA.get_states();
    auto right_transition_table=right_NFA.get_transition_table();

    left_states.merge(right_states);
    left_transition_table.merge(right_transition_table);
    
    return {left_states, alphabet->name(),start_state,left_transition_table,right_NFA.get_final_states()};

  } else {
    
    auto inner_start_state=start_state+1;
    auto inner_NFA=to_NFA(tree->left_node,inner_start_state);
    auto inner_states=inner_NFA.get_states();
    auto inner_final_states=inner_NFA.get_final_states();
    auto inner_transition_table=inner_NFA.get_transition_table();
    inner_states.insert(start_state);
    auto final_state= inner_start_state+inner_states.size();
    inner_states.insert(final_state);

    inner_transition_table[{    start_state ,alphabet->get_epsilon()  }]={inner_start_state,final_state};
    for(auto const & inner_final_state:inner_final_states) {
      inner_transition_table[{    inner_final_state,alphabet->get_epsilon()  }]={inner_start_state,final_state};
    }

    return {inner_states, alphabet->name(),start_state,inner_transition_table,{final_state}};
  }

}

NFA regex::to_NFA() const  {
  return to_NFA(syntax_tree,0);
}

} // namespace cyy::lang
