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

class CFG_production final {

public:
  using head_type = grammar_symbol_type::nonterminal_type;
  using body_type = std::vector<grammar_symbol_type>;

  CFG_production(head_type head_,body_type body_):head{std::move(head_)},body{std::move(body_)} {
  }

  CFG_production(const CFG_production &) = default;
  CFG_production &operator=(const CFG_production &) = default;

  CFG_production(CFG_production &&) noexcept = default;
  CFG_production &operator=(CFG_production &&) noexcept = default;

  ~CFG_production() = default;

  bool is_epsilon() const {
    return body.size() == 1 && is_epsilon(body[0]);
  }

  bool is_epsilon(const grammar_symbol_type &grammal_symbol) const {
    auto terminal_ptr = grammal_symbol.get_terminal_ptr();
    return terminal_ptr && alphabet->is_epsilon(*terminal_ptr);
  }

protected:
  void print(std::ostream &os, std::string_view alphabet_name) const {
    os << head << " -> ";
    for (const auto &grammal_symbol : body) {
      print(os,alphabet_name);
      os << ' ';
    }
    os << '\n';
    return;
  }

private:
  grammar_symbol_type::nonterminal_type head;
  body_type body;
};

} // namespace cyy::computation
