/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <map>
#include <string>
#include <variant>
#include <vector>

#include "automata.hpp"
#include "lang.hpp"

namespace cyy::lang {

class CFG {

public:
  using terminal_type = symbol_type;
  using nonterminal_type = std::string;
  using grammar_symbol_type = std::variant<terminal_type, nonterminal_type>;
  using production_body_type = std::vector<grammar_symbol_type>;

  CFG(const std::string &alphabet_name, const nonterminal_type &start_symbol_,
      std::map<nonterminal_type, std::vector<production_body_type>>
          productions_)
      : alphabet(make_alphabet(alphabet_name)), start_symbol(start_symbol_),
        productions(productions_) {

	  eliminate_useless_symbols();

    bool has_start_symbol = false;
    for (auto const &[head, bodies] : productions) {
      if (!has_start_symbol && head == start_symbol) {
        has_start_symbol = true;
      }

      if (bodies.empty()) {
        throw std::invalid_argument(std::string("no body for head ") + head);
      }

      for (auto const &body : bodies) {
        if (body.empty()) {
          throw std::invalid_argument(std::string("an empty body for head ") +
                                      head);
        }
        for (auto const &symbol : body) {
          if (std::holds_alternative<terminal_type>(symbol)) {
            auto terminal = std::get<terminal_type>(symbol);
            if (!alphabet->contain(terminal)) {
              throw std::invalid_argument(std::string("invalid terminal ") +
                                          std::to_string(terminal));
            }
          }
        }
      }
    }
    if(!has_start_symbol) {
        throw std::invalid_argument("no productions for start symbol");
    }
  }

  bool operator==(const CFG &rhs) const {
    return (this == &rhs) || ( alphabet->name()==rhs.alphabet->name() && start_symbol == rhs.start_symbol && productions == rhs.productions);

  }
  void  print(std::ostream& os) const {
    //by convention,we print start symbol first.
    auto it=productions.find(start_symbol);
    for(auto const & body:it->second) {
      print(os,start_symbol,body);
    }
    for(auto const &[head,bodys]:productions) {
      if(head ==start_symbol) {
	continue;
      }
    for(auto const & body:bodys) {
      print(os,head,body);
    }
    }
  }


  auto get_alphabet() const -> auto const & { return *alphabet; }

  void eliminate_useless_symbols();

  void eliminate_left_recursion();

  void left_factoring();

private:
   void print(std::ostream& os,const nonterminal_type & head,const production_body_type &body) const {

      os<<head<<" -> ";
    for(const auto &grammal_symbol:body) {

      if(auto ptr = std::get_if<terminal_type>(&grammal_symbol))
	alphabet->print(os,*ptr);
      else {
	os << std::get<nonterminal_type>(grammal_symbol);
      }
      os<<' ';
    }
      os<<'\n';
    return;

  }

  std::set<nonterminal_type>
  left_factoring_nonterminal(const nonterminal_type &head);

  nonterminal_type get_new_head(const nonterminal_type &head) {
    auto new_head = head + "'";
    while (productions.find(new_head) != productions.end()) {
      new_head += "'";
    }
    return new_head;
  }

 // std::map <grammar_symbol_type, std::set<terminal_type> first() const;

private:
  std::unique_ptr<ALPHABET> alphabet;
  nonterminal_type start_symbol;
  std::map<nonterminal_type, std::vector<production_body_type>> productions;
};

} // namespace cyy::lang
