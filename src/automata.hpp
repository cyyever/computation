/*!
 * \file automata.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <map>
#include <set>
#include <string>

namespace cyy::compiler {

  class NFA  {
    public:
      using state_type=uint64_t;
      using symbol_type=uint64_t;

      NFA(
	  const std::set<state_type> &states_,
	  const std::set<symbol_type>& alphabet_,
	  const state_type & start_state_,
	  const std::map<std::pair<state_type,symbol_type>,std::set<state_type>> & transition_table_,
	  const std::set<state_type> & final_states_):states(states_),alphabet(alphabet_),start_state(start_state_),final_states(final_states_),transition_table(transition_table_) {

	if(states.empty()) {
	  throw  std::invalid_argument("no state");
	}
	if(alphabet.empty()) {
	  throw  std::invalid_argument("no alphabet");
	}
	if(alphabet.count(epsilon)) {
	  throw  std::invalid_argument("alphabet contains epsilon");
	}
	if(!states.count(start_state)) {
	  throw  std::invalid_argument("unexisted start state");
	}
	for(auto const &final_state:final_states) {
	if(!states.count(final_state)) {
	  throw  std::invalid_argument(std::string("unexisted start state ")+std::to_string(final_state));
	}
	}
      }

      NFA(const NFA&) = default;
      NFA& operator=(const NFA&) = default;

      NFA(NFA&&) noexcept = default;
      NFA& operator=(NFA&&) noexcept = default;



      state_type get_start_state() const {return start_state;}

      auto get_alphabet() const -> auto const  & {
	return alphabet;
      }


       bool  contain_final_state(const std::set<state_type> &T) const {
	 for(const auto &f:final_states) {
	   if(T.count(f)==1) {
	     return true;
	   }
	 }
	 return false;
       }
      std::set<state_type> epsilon_closure (const std::set<state_type> &T) const {
	auto stack=T;
	auto res=T;
	while(!stack.empty()) {
	  decltype(stack) next_stack;
	  for(auto const &t:res) {
	    auto it=transition_table.find({t,epsilon});
	    if(it==transition_table.end())  {
	      continue;
	    }
	      for(auto const &u:it->second) {
		if(res.count(u)==0) {
		  next_stack.insert(u);
		  res.insert(u);
		}
	      }
	  }
	  stack=next_stack;
	}
	return res;
      }

      std::set<state_type> move(const std::set<state_type> & T,symbol_type a)  const {
	std::set<state_type> direct_reachable;


	for(const auto &s:T) {
	    auto it=transition_table.find({s,a});
	    if(it!=transition_table.end()) {
	      direct_reachable.insert(it->second.begin(),it->second.end());
	    }
	}
	
	return epsilon_closure(direct_reachable);
      }

    public:
      static constexpr symbol_type epsilon=0;


    protected:
      std::set<state_type> states;
      std::set<symbol_type> alphabet;
      state_type start_state;
      std::set<state_type> final_states;
    private:
      std::map<std::pair<state_type,symbol_type>,std::set<state_type>> transition_table;
  };

  class DFA final:public NFA {
    public:

      DFA(
	  std::set<state_type> states_,
	  std::set<symbol_type> alphabet_,
	  state_type start_state_,
	  std::map<std::pair<state_type,symbol_type>,state_type> transition_table_,
	  std::set<state_type> final_states_):NFA(states_,alphabet_,start_state_,{},final_states_),transition_table{transition_table_} {

	for(auto const &s:states) {
	  for(auto const &a:alphabet) {
	    if(transition_table.find({s,a})==transition_table.end()) {
	      throw  std::invalid_argument(std::string("no transition for state ")+std::to_string(s)+" and symbol "+std::to_string(a));
	    }
	  }
	}
	if(transition_table.size()!=states.size()* alphabet.size()) {
	  throw  std::invalid_argument("invalid transition table");
	}
      }

      DFA(const DFA&) = default;
      DFA& operator=(const DFA&) = default;

      DFA(DFA&&) noexcept = default;
      DFA& operator=(DFA&&) noexcept = default;

      state_type move(state_type s, symbol_type a) {
	return transition_table[{s,a}];
      }

    private:
      std::map<std::pair<state_type,symbol_type>,state_type> transition_table;
  };

  DFA NFA_to_DFA(const NFA &nfa) ;

}
