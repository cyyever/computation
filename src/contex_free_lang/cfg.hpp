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
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "../formal_grammar/grammar.hpp"
#include "../lang/lang.hpp"

namespace cyy::computation {

class CFG {

public:
  using terminal_type = grammar_symbol_type::terminal_type;
  using nonterminal_type = grammar_symbol_type::nonterminal_type;
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

  CFG(const std::string &alphabet_name, nonterminal_type start_symbol_,
      std::map<nonterminal_type, std::vector<production_body_type>>
          productions_);

  CFG(const CFG &) = default;
  CFG &operator=(const CFG &) = default;

  CFG(CFG &&) noexcept = default;
  CFG &operator=(CFG &&) noexcept = default;

  virtual ~CFG() = default;

  bool operator==(const CFG &rhs) const;

  void print(std::ostream &os) const;

  bool has_production(const production_type &production) const;

  std::set<nonterminal_type> get_heads() const;

  auto get_alphabet() const noexcept -> const auto & { return alphabet; }

  auto get_productions() const noexcept -> const auto & { return productions; }

  std::set<terminal_type> get_terminals() const {
    std::set<terminal_type> terminals;
    for (auto const &[_, bodies] : productions) {
      for (auto const &body : bodies) {
        for (auto const &symbol : body) {
          if (auto ptr = symbol.get_terminal_ptr();
              ptr && !alphabet->is_epsilon(*ptr)) {
            terminals.insert(*ptr);
          }
        }
      }
    }
    return terminals;
  }

  void eliminate_useless_symbols();

  void eliminate_left_recursion(std::vector<nonterminal_type> old_heads = {});

  void eliminate_epsilon_productions();

  void eliminate_single_productions();

  //! brief covert grammar to Chomsky normal form
  void to_CNF();

  void left_factoring();

  bool recursive_descent_parse(symbol_string_view view) const;

  const std::map<nonterminal_type, std::set<terminal_type>> &first() const;

  std::map<nonterminal_type, std::set<terminal_type>> follow() const;

  bool is_CNF() const;

  std::set<nonterminal_type> nullable() const;

  bool is_epsilon(const production_body_type &body) const {
    return body.size() == 1 && is_epsilon(body[0]);
  }

  bool is_epsilon(const grammar_symbol_type &grammal_symbol) const {
    auto terminal_ptr = grammal_symbol.get_terminal_ptr();
    return terminal_ptr && alphabet->is_epsilon(*terminal_ptr);
  }

  std::set<terminal_type>
  first(const grammar_symbol_const_span_type &alpha) const;

protected:
  void print(std::ostream &os, const terminal_type &terminal) const {
    alphabet->print(os, terminal);
  }
  void print(std::ostream &os,
             const grammar_symbol_type &grammal_symbol) const {
    if (auto ptr = grammal_symbol.get_terminal_ptr())
      alphabet->print(os, *ptr);
    else {
      os << *(grammal_symbol.get_nonterminal_ptr());
    }
  }

  void print(std::ostream &os, const nonterminal_type &head,
             const production_body_type &body) const {

    os << head << " -> ";
    for (const auto &grammal_symbol : body) {
      print(os, grammal_symbol);
      os << ' ';
    }
    os << '\n';
    return;
  }

  nonterminal_type get_new_head(nonterminal_type advise_head) const {
    do {
      advise_head.push_back('\'');
    } while (productions.count(advise_head));
    return advise_head;
  }

  static nonterminal_type
  get_new_head(nonterminal_type advise_head,
               const std::set<nonterminal_type> &heads) {
    do {
      advise_head.push_back('\'');
    } while (heads.count(advise_head));
    return advise_head;
  }

  void normalize_productions();

protected:
  std::shared_ptr<ALPHABET> alphabet;
  nonterminal_type start_symbol;
  std::map<nonterminal_type, std::vector<production_body_type>> productions;

private:
  mutable std::map<nonterminal_type, std::set<terminal_type>> first_sets;
};

} // namespace cyy::computation
