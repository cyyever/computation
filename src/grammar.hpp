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
  }
  auto get_alphabet() const -> auto const & { return *alphabet; }

  void eliminate_useless_symbols();

  void eliminate_left_recursion();

  void left_factoring();

private:
  std::set<nonterminal_type>
  left_factoring_nonterminal(const nonterminal_type &head);

  nonterminal_type get_new_head(const nonterminal_type &head) {
    auto new_head = head + "'";
    while (productions.find(new_head) != productions.end()) {
      new_head += "'";
    }
    return new_head;
  }

private:
  std::unique_ptr<ALPHABET> alphabet;
  nonterminal_type start_symbol;
  std::map<nonterminal_type, std::vector<production_body_type>> productions;
};

} // namespace cyy::lang
