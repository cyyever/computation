/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "grammar.hpp"

namespace cyy::lang {

void CFG::eliminate_useless_symbols() {
  if (productions.empty()) {
    return;
  }

  enum class nonterminal_state {unchecked, checking, useless, non_useless };

  std::map<nonterminal_type, nonterminal_state> states;
  std::map<nonterminal_type, std::set<nonterminal_type>> depedency_heads;

  for(const auto &[head,_]:productions) {
    states[head]=nonterminal_state::unchecked;
  }

  auto check_nonterminal =
      [&](auto &&self, const nonterminal_type &head, CFG &cfg
	) ->void{

	states[head] = nonterminal_state::checking;

      auto &bodies = cfg.productions[head];
      for (auto &body : bodies) {
        bool useless = false;
        for (auto const &symbol : body) {
          if (!std::holds_alternative<nonterminal_type>(symbol)) {
            continue;
          }
          auto nonterminal = std::get<nonterminal_type>(symbol);
          auto it = states.find(nonterminal);
          if (it == states.end()) {
            body.clear();
            useless = true;
            break;
	  }
          if (it->second == nonterminal_state::unchecked) {
	    self(self,nonterminal,cfg);
	    it = states.find(nonterminal);
          }
          if (it->second == nonterminal_state::useless) {
            body.clear();
            useless = true;
            break;
          } else if (it->second == nonterminal_state::checking) {
	    depedency_heads[nonterminal].insert(head);
            useless = true;
          }
        }
        if (!useless) {
          states[head] = nonterminal_state::non_useless;
        }
      }
      bodies.erase(
          std::remove_if(bodies.begin(), bodies.end(),
                         [](const auto &body) { return body.empty(); }),
          bodies.end());
      if (bodies.empty()) {
        states[head] = nonterminal_state::useless;
      }

      if(states[head]!=nonterminal_state::checking) {
	for(auto const &depedency_head:depedency_heads[head]) {
	  if(states[depedency_head]==nonterminal_state::checking) {
	  states[depedency_head]=nonterminal_state::unchecked;
	  self(self,depedency_head,cfg);
	  }
	}
      }
  };

  decltype(productions) new_productions;
  check_nonterminal(check_nonterminal, start_symbol, *this);

  auto add_nonterminal =
      [&](auto &&self, const nonterminal_type &head, CFG &cfg 
	 )->void {
    auto &bodies = cfg.productions[head];
    for (auto &body : bodies) {
      if(body.empty()) {
	continue;
      }
      bool add = true;
      std::set<nonterminal_type> nonterminals;
      for (auto const &symbol : body) {
        if (!std::holds_alternative<nonterminal_type>(symbol)) {
          continue;
        }
        auto nonterminal = std::get<nonterminal_type>(symbol);
	if(states[nonterminal] != nonterminal_state::non_useless) {
          add = false;
          break;
        }
	nonterminals.insert(nonterminal);
      }
      if (!add) {
        continue;
      }
      new_productions[head].emplace_back(std::move(body));
      body.clear();
      for (auto const &nonterminal:nonterminals) {
        self(self, nonterminal, cfg);
      }
    }
  };

  add_nonterminal(add_nonterminal, start_symbol, *this);
  productions = new_productions;
}

void CFG::eliminate_left_recursion() {

  std::vector<nonterminal_type> old_heads{start_symbol};

  for(const auto &[head,_]:productions) {
    if(head!=start_symbol) {
    old_heads.push_back(head);
    }
  }

  auto eliminate_immediate_left_recursion =
      [this](const nonterminal_type &head) {

        auto new_head = get_new_head(head);
	std::vector<production_body_type> new_bodies;
        auto &bodies =productions[head];

        for (auto &body :bodies) {

	  if (std::holds_alternative<nonterminal_type>(body.front()) && std::get<nonterminal_type>(body.front()) == head) {
            body.erase(body.begin());
            body.emplace_back(new_head);
            new_bodies.emplace_back(std::move(body));
	    body.clear();
	  }
        }

	if(new_bodies.empty()) {
	  return;
	}
	new_bodies.emplace_back(  1,symbol_type( alphabet->get_epsilon()) );
	productions[new_head]=new_bodies;

        for (auto &body :bodies) {
	  if(!body.empty()) {
	    body.push_back(new_head);
	  }

	}
      };

  for(size_t i=0;i<old_heads.size();i++) {
    for(size_t j=0;j<i;j++) {
      std::vector<production_body_type> new_bodies;
    for (auto &body :productions[old_heads[i]]) {

      if (!(std::holds_alternative<nonterminal_type>(body.front()) && std::get<nonterminal_type>(body.front()) ==old_heads[j]) ) {
	new_bodies.emplace_back(std::move(body));
	continue;
      }
      // Ai -> Aj，替换
      for(const auto & head_j_body:productions[old_heads[j]]) {
	auto new_body=head_j_body;
	  new_body.insert(new_body.end(), body.begin() + 1, body.end());
	new_bodies.push_back(new_body);
      }
    }
    productions[old_heads[i]]=std::move(new_bodies);
    }
    eliminate_immediate_left_recursion(old_heads[i]);
  }
    normalize_productions();
  return;
}

std::set<CFG::nonterminal_type>
CFG::left_factoring_nonterminal(const nonterminal_type &head) {
  if (productions.empty()) {
    return {};
  }

  auto it=productions.find(head);
  if(it==productions.end()) {
    return {};
  }

  auto &bodies = it->second;
  if (bodies.size() < 2) {
    return {};
  }
  std::sort(bodies.begin(), bodies.end());
  auto common_prefix = bodies.front();
  bool is_common_prefix = false;
  std::vector<decltype(bodies.begin())> iterators;
  for (auto it = bodies.begin() + 1; it != bodies.end(); it++) {
    size_t i = 0;
    for (; i < common_prefix.size() && i < it->size(); i++) {
      if (common_prefix[i] != (*it)[i]) {
        break;
      }
    }
    if (i > 0) {
      is_common_prefix = true;
      common_prefix.resize(i);
      iterators.push_back(it);
      continue;
    }
    break;
  }
  if (is_common_prefix) {
    auto new_head = get_new_head(head);
    for (auto &it : iterators) {
      productions[new_head].emplace_back(it->begin() + common_prefix.size(),
                                         it->end());
      it->clear();
    }

    bodies.erase(std::remove_if(bodies.begin(), bodies.end(),
                                [](const auto &body) { return body.empty(); }),
                 bodies.end());
    common_prefix.push_back(new_head);
    bodies.push_back(common_prefix);
    auto res = left_factoring_nonterminal(head);
    res.insert(new_head);
    return res;
  }
  return {};
}

void CFG::left_factoring() {
  if (productions.empty()) {
    return;
  }

  std::set<nonterminal_type> new_nonterminals;
  for (auto const &[head, _] : productions) {
    new_nonterminals.insert(head);
  }

  while (!new_nonterminals.empty()) {
    auto nonterminals = std::move(new_nonterminals);
    new_nonterminals.clear();
    for (auto const &nonterminal : nonterminals) {
      new_nonterminals.merge(left_factoring_nonterminal(nonterminal));
    }
  }
}

/*
std::map < CFG::grammar_symbol_type,
    std::set<CFG::terminal_type>> CFG::first() const {


  std::map < CFG::grammar_symbol_type, std::set<CFG::terminal_type> first_sets;
  auto  first_of_grammar_symbol=
      [](auto &&self, const grammar_symbol_type &grammar_symbol, CFG &cfg,
  std::map < CFG::grammar_symbol_type, std::set<CFG::terminal_type> &first_sets
	 ) {

	if (std::holds_alternative<terminal_type>(grammar_symbol)) {
	  first_sets[grammar_symbol].insert(std::get<terminal_type>(grammar_symbol));
	  return;
	}

  auto nonterminal = std::get<nonterminal_type>(grammar_symbol);
  auto it=productions.find(nonterminal);
  if(it==productions.end()) {
    return;
  }

  while (true) {
    auto cur_first_sets = first_sets;
      for (auto &body : *it) {
        for (size_t i = 0; i < body.size(); i++) {
          if (std::holds_alternative<terminal_type>(body[i])) {
            cur_first_sets[head].insert(std::get<terminal_type>(body[i]));
            break;
          }
          auto nonterminal = std::get<nonterminal_type>(symbol);
          if (cur_first_sets[nonterminal].count(alphabet->get_epsilon()) == 0) {
            break;
          }
        }
        if (i == body.size()) {
          cur_first_sets[nonterminal].insert(alphabet->get_epsilon());
        }
      }
    if (cur_first_sets == first_sets) {
      break;
    }
  }

return first_sets;
} // namespace cyy::lang
*/

CFG NFA_to_CFG(const NFA &nfa) {
  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      productions;

  auto state_to_nonterminal = [](symbol_type state) {
    return std::string("S") + std::to_string(state);
  };

  for (auto const &[p, next_states] : nfa.get_transition_table()) {
    auto const &[cur_state, symbol] = p;
    for (auto const &next_state : next_states) {
      if (symbol != nfa.get_alphabet().get_epsilon()) {
        productions[state_to_nonterminal(cur_state)].push_back(
            CFG::production_body_type{{symbol},
                                      {state_to_nonterminal(next_state)}});
      } else {
        productions[state_to_nonterminal(cur_state)].push_back(
            CFG::production_body_type{{state_to_nonterminal(next_state)}});
      }
    }
  }

  for (auto const &final_state : nfa.get_final_states()) {
    productions[state_to_nonterminal(final_state)].push_back(
        CFG::production_body_type{{nfa.get_alphabet().get_epsilon()}});
  }

  return {nfa.get_alphabet().name(),
          state_to_nonterminal(nfa.get_start_state()), productions};
}

} // namespace cyy::lang
