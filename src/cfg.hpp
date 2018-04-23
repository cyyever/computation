/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <algorithm>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "automaton.hpp"
#include "lang.hpp"

namespace cyy::lang {

class CFG {

public:
  using terminal_type = symbol_type;
  using nonterminal_type = std::string;
  using grammar_symbol_type = std::variant<terminal_type, nonterminal_type>;
  using grammar_symbol_string_view =
      std::basic_string_view<grammar_symbol_type>;
  using production_body_type = std::vector<grammar_symbol_type>;
  using production_type = std::pair<nonterminal_type, production_body_type>;

  struct parse_node;
  using parse_node_ptr = std::shared_ptr<parse_node>;
  struct parse_node {
    explicit parse_node(grammar_symbol_type grammar_symbol_)
        : grammar_symbol(std::move(grammar_symbol_)) {}

    grammar_symbol_type grammar_symbol;

    std::vector<parse_node_ptr> children;
  };

  CFG(const std::string &alphabet_name, const nonterminal_type &start_symbol_,
      std::map<nonterminal_type, std::vector<production_body_type>>
          &productions_);

  bool operator==(const CFG &rhs) const {
    return (this == &rhs) ||
           (alphabet->name() == rhs.alphabet->name() &&
            start_symbol == rhs.start_symbol && productions == rhs.productions);
  }
  void print(std::ostream &os) const {
    // by convention,we print start symbol first.
    auto it = productions.find(start_symbol);
    for (auto const &body : it->second) {
      print(os, start_symbol, body);
    }
    for (auto const &[head, bodys] : productions) {
      if (head == start_symbol) {
        continue;
      }
      for (auto const &body : bodys) {
        print(os, head, body);
      }
    }
  }

  std::set<nonterminal_type> get_heads() const {
    std::set<nonterminal_type> heads;
    for (auto const &[head, _] : productions) {
      heads.insert(head);
    }
    return heads;
  }

  std::set<terminal_type> get_terminals() const {
    std::set<terminal_type> terminals;
    for (auto const &[_, bodies] : productions) {
      for (auto const &body : bodies) {
        for (auto const &symbol : body) {
          if (auto ptr = std::get_if<terminal_type>(&symbol);
              ptr && !alphabet->is_epsilon(*ptr)) {
            terminals.insert(*ptr);
          }
        }
      }
    }
    return terminals;
  }

  void normalize_productions() {
    decltype(productions) new_productions;
    for (auto &[head, bodies] : productions) {
      std::set<production_body_type> bodies_set;
      for (auto &body : bodies) {
        if (body.empty()) {
          continue;
        }

        auto it = std::remove_if(body.begin(), body.end(),
                                 [this](const auto &grammal_symbol) {
                                   return is_epsilon(grammal_symbol);
                                 });
        if (it > body.begin()) {
          bodies_set.emplace(std::move_iterator(body.begin()),
                             std::move_iterator(it));
        } else {
          bodies_set.emplace(1, symbol_type(alphabet->get_epsilon()));
        }
      }
      if (!bodies_set.empty()) {
        new_productions[head] = {std::move_iterator(bodies_set.begin()),
                                 std::move_iterator(bodies_set.end())};
      }
    }
    productions = std::move(new_productions);
  }
  void eliminate_useless_symbols();

  void eliminate_left_recursion(std::vector<nonterminal_type> old_heads = {});

  void eliminate_epsilon_productions();

  void eliminate_single_productions();

  //! brief covert grammar to Chomsky normal form
  void to_CNF();

  void left_factoring();

  bool recursive_descent_parse(symbol_string_view view) const;

  std::map<nonterminal_type, std::set<terminal_type>> first() const;

  std::map<nonterminal_type, std::set<terminal_type>> follow() const;

  bool is_CNF() const;

  std::set<nonterminal_type> nullable() const;

protected:
  void print(std::ostream &os, const nonterminal_type &head,
             const production_body_type &body) const {

    os << head << " -> ";
    for (const auto &grammal_symbol : body) {

      if (auto ptr = std::get_if<terminal_type>(&grammal_symbol))
        alphabet->print(os, *ptr);
      else {
        os << std::get<nonterminal_type>(grammal_symbol);
      }
      os << ' ';
    }
    os << '\n';
    return;
  }

  nonterminal_type get_new_head(nonterminal_type advise_head) const {
    advise_head.push_back('\'');

    while (productions.find(advise_head) != productions.end()) {
      advise_head.push_back('\'');
    }
    return advise_head;
  }

  static nonterminal_type
  get_new_head(nonterminal_type advise_head,
               const std::set<nonterminal_type> &heads) {
    advise_head.push_back('\'');

    while (heads.count(advise_head)) {
      advise_head.push_back('\'');
    }
    return advise_head;
  }

  bool is_epsilon(const grammar_symbol_type &grammal_symbol) const {
    auto terminal_ptr = std::get_if<terminal_type>(&grammal_symbol);
    return terminal_ptr && alphabet->is_epsilon(*terminal_ptr);
  }

  std::set<terminal_type>
  first(const grammar_symbol_string_view &alpha,
        const std::map<nonterminal_type, std::set<terminal_type>>
            &nonterminal_first_sets) const;

  std::map<nonterminal_type, std::set<terminal_type>>
  follow(const std::map<nonterminal_type, std::set<terminal_type>>
             &nonterminal_first_sets) const;

protected:
  std::shared_ptr<ALPHABET> alphabet;
  nonterminal_type start_symbol;
  std::map<nonterminal_type, std::vector<production_body_type>> productions;
};
} // namespace cyy::lang
