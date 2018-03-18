/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "grammar.hpp"

namespace cyy::lang {

  void CFG::eliminate_useless_symbols() {
    if(productions.empty()) {
      return;
    }

    enum class nonterminal_state {
      checking,
      useless,
      non_useless
    };

    auto check_nonterminal=[](auto && self,const nonterminal_type &head,CFG & cfg,std::map<nonterminal_type,nonterminal_state>& states) ->void {
      states[head]=nonterminal_state::checking;

      while(true) {
	auto prev_states=states;

	auto &bodies=cfg.productions[head];
	for(auto &body:bodies) {
	  bool useless=false;
	  for(auto const &symbol:body) {
	    if(!std::holds_alternative<nonterminal_type>(symbol)) {
	      continue;
	    }
	    auto nonterminal=std::get<nonterminal_type>(symbol);
	    auto it=states.find(nonterminal);
	    if(it==states.end()) {
	      self(self,nonterminal,cfg,states);
	      it=states.find(nonterminal);
	    }
	    if(it->second==nonterminal_state::useless) {
	      body.clear();
	      useless=true;
	      break;
	    }else if(it->second==nonterminal_state::checking) {
	      useless=true;
	    }
	  }
	  if(!useless) {
	    states[head]=nonterminal_state::non_useless;
	  }
	}
	bodies.erase(std::remove_if(bodies.begin(), bodies.end(), [](const auto& body){return body.empty();}), bodies.end());
	if(bodies.empty()) {
	    states[head]=nonterminal_state::useless;
	}
	if(prev_states==states) {
	  return;
	}
      }
    };

    std::map<nonterminal_type,nonterminal_state> states;
    check_nonterminal(check_nonterminal,start_symbol,*this,states);

    auto add_nonterminal=[](auto && self,const nonterminal_type &head,CFG & cfg,const std::map<nonterminal_type,nonterminal_state>& states,decltype(CFG::productions) & new_productions  ) ->void {
	auto &bodies=cfg.productions[head];
	for(auto &body:bodies) {
	  bool add=true;
	  for(auto const &symbol:body) {
	    if(!std::holds_alternative<nonterminal_type>(symbol)) {
	      continue;
	    }
	    auto nonterminal=std::get<nonterminal_type>(symbol);
	    auto it=states.find(nonterminal);
	    if(it==states.end() || it->second!=nonterminal_state::non_useless) {
	      add=false;
	      break;
	    }
	  }
	  if(!add) {
	    continue;
	  }
	  for(auto const &symbol:body) {
	    if(!std::holds_alternative<nonterminal_type>(symbol)) {
	      continue;
	    }
	    auto nonterminal=std::get<nonterminal_type>(symbol);
	    self(self,nonterminal,cfg,states,new_productions);
	  }
	  new_productions[head].emplace_back(std::move(body));
	}
    };

    decltype(productions) new_productions;

    add_nonterminal(add_nonterminal,start_symbol,*this,states,new_productions);
    productions=new_productions;
  }

void CFG::eliminate_left_recursion() {

  auto eliminate_immediate_left_recursion=[this](const nonterminal_type & head) {
    auto &this_bodies=productions[head];
    auto bodies=std::move(this_bodies);
    this_bodies.clear();

    auto new_head=get_new_head(head);
    auto &new_bodies=productions[new_head];

    for(auto &body:bodies) {
      if(std::holds_alternative<nonterminal_type>(body.front()) && std::get<nonterminal_type>(body.front()) == head  ) {
	body.erase(body.begin());
	body.emplace_back(new_head);
	new_bodies.push_back(body);
      } else {
	body.emplace_back(new_head);
	this_bodies.push_back(body);
      }
    }
  };

  for(auto &[head,bodies]:productions) {
      auto this_bodies=std::move(bodies);
      bodies.clear();
      for(auto &body:this_bodies) {

	//Ai -> Aj 
	if(std::holds_alternative<terminal_type>(body.front())) {
	  bodies.emplace_back(std::move(body));
	  continue;
	}

	auto first_nonterminal=std::get<nonterminal_type>(body.front());
	if(first_nonterminal>=head) {
	  bodies.emplace_back(std::move(body));
	  continue;
	}
	auto it=productions.find(first_nonterminal);
	if(it==productions.end()) {
	  bodies.emplace_back(std::move(body));
	  continue;
	}

	for(auto const &first_nonterminal_body:it->second) {
	  bodies.push_back(first_nonterminal_body);
	  bodies.back().insert(bodies.back().end(),body.begin()+1,body.end());
	}
      }
     eliminate_immediate_left_recursion(head);
  }
  return;
}

std::set<CFG::nonterminal_type> CFG::left_factoring_nonterminal(const nonterminal_type & head) {
    auto &bodies=productions[head];
    if(bodies.size()<2) {
      return {};
    }
    std::sort(bodies.begin(),bodies.end());
    auto common_prefix=bodies.front();
    bool is_common_prefix=false;
    std::vector<decltype(bodies.begin())> iterators;
    for(auto it=bodies.begin()+1 ;it!=bodies.end();it++) {
      size_t i=0;
      for(;i<common_prefix.size()&& i<it->size();i++) {
	if(common_prefix[i]!=(*it)[i]) {
	  break;
	}
      }
      if(i>0) {
	is_common_prefix=true;
	common_prefix.resize(i);
	iterators.push_back(it);
	continue;
      }
      break;
    }
    if(is_common_prefix) {
      auto new_head=get_new_head(head);
      for(auto &it:iterators) {
	productions[new_head].emplace_back(it->begin()+common_prefix.size(),it->end());
	it->clear();
      }

      bodies.erase(std::remove_if(bodies.begin(), bodies.end(), [](const auto& body){return body.empty();}), bodies.end());
      common_prefix.push_back(new_head);
      bodies.push_back(common_prefix);
      auto res=left_factoring_nonterminal(head);
      res.insert(new_head);
      return res;
    }
    return {};
}

void CFG::left_factoring() {

  std::set<nonterminal_type> new_nonterminals;
  for(auto   const &   [head,_]:productions) {
    new_nonterminals.insert(head);
  }

  while(!new_nonterminals.empty()) {
    auto nonterminals=std::move(new_nonterminals);
    new_nonterminals.clear();
    for(auto const &nonterminal: nonterminals) {
      new_nonterminals.merge(left_factoring_nonterminal(nonterminal));
    }
  }
}

CFG NFA_to_CFG(const NFA &nfa) {
  std::map<CFG::nonterminal_type,std::vector<CFG::production_body_type>>  productions;

  auto state_to_nonterminal = [](symbol_type state) {
    return std::string("S") + std::to_string(state);
  };

  for (auto const &[p, next_states] : nfa.get_transition_table()) {
    auto const &[cur_state, symbol] = p;
    for (auto const &next_state : next_states) {
      if (symbol != nfa.get_alphabet().get_epsilon()) {
        productions[
            state_to_nonterminal(cur_state)].push_back(
	    CFG::production_body_type{{symbol},
                                           {state_to_nonterminal(next_state)}});
      } else {
        productions[
            state_to_nonterminal(cur_state)].push_back(
	    CFG::production_body_type{{state_to_nonterminal(next_state)}});
      }
    }
  }

  for (auto const &final_state : nfa.get_final_states()) {
    productions[
        state_to_nonterminal(final_state)].push_back(
	    CFG::production_body_type{{nfa.get_alphabet().get_epsilon()}});
  }

  return {nfa.get_alphabet().name(),
          state_to_nonterminal(nfa.get_start_state()), productions};
}

} // namespace cyy::lang
