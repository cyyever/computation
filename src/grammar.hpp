/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <variant>
#include <vector>

#include "lang.hpp"

namespace cyy::lang {

class CFG {

public:
  using terminal_type = symbol_type;
  using nonterminal_type = std::string;

  struct production {
    nonterminal_type head;

    std::vector<

        std::variant<terminal_type, nonterminal_type>

        >
        body;
  };

  CFG(const std::string &alphabet_name, const nonterminal_type &start_symbol_,
      std::vector<production> productions_)
      : alphabet(make_alphabet(alphabet_name)), start_symbol(start_symbol_),
        productions(productions_) {
    bool has_start_symbol = false;
    for (auto const &[head, body] : productions) {
      if (!has_start_symbol && head == start_symbol) {
        has_start_symbol = true;
      }

      for (auto const &elem : body) {
        if (std::holds_alternative<terminal_type>(elem)) {
          auto terminal = std::get<terminal_type>(elem);
          if (!alphabet->contain(terminal)) {
            throw std::invalid_argument(std::string("invalid symbol ") +
                                        std::to_string(terminal));
          }
        }
      }
    }
  }
  auto get_alphabet() const -> auto const & { return *alphabet; }

private:
  std::unique_ptr<ALPHABET> alphabet;
  nonterminal_type start_symbol;
  std::vector<production> productions;
};

} // namespace cyy::lang
