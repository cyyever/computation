/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

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
  using element_type = std::variant<terminal_type, nonterminal_type>;
  struct production {
    nonterminal_type head{};
    std::vector<element_type> body;
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

CFG NFA_to_CFG(const NFA &nfa) {
  std::vector<CFG::production> productions;

  auto state_to_nonterminal = [](symbol_type state) {
    return std::string("S") + std::to_string(state);
  };

  for (auto const &[p, next_states] : nfa.get_transition_table()) {
    auto const &[cur_state, symbol] = p;
    for (auto const &next_state : next_states) {
      if (symbol != nfa.get_alphabet().get_epsilon()) {
        productions.emplace_back(
            state_to_nonterminal(cur_state),
            std::vector<CFG::element_type>{{symbol},
                                           {state_to_nonterminal(next_state)}});
      } else {
        productions.emplace_back(
            state_to_nonterminal(cur_state),
            std::vector<CFG::element_type>{{state_to_nonterminal(next_state)}});
      }
    }
  }

  for (auto const &final_state : nfa.get_final_states()) {
    productions.emplace_back(
        state_to_nonterminal(final_state),
        std::vector<CFG::element_type>{{nfa.get_alphabet().get_epsilon()}});
  }

  return {nfa.get_alphabet().name(),
          state_to_nonterminal(nfa.get_start_state()), productions};
}

} // namespace cyy::lang
