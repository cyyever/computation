/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <iosfwd>

#include "../formal_grammar/grammar.hpp"

namespace cyy::computation {

class CFG_production final {

public:
  using head_type = grammar_symbol_type::nonterminal_type;
  using body_type = grammar_symbol_string_type;

  CFG_production(head_type head_, body_type body_)
      : head{std::move(head_)}, body{std::move(body_)} {}

  CFG_production(const CFG_production &) = default;
  CFG_production &operator=(const CFG_production &) = default;

  CFG_production(CFG_production &&) noexcept = default;
  CFG_production &operator=(CFG_production &&) noexcept = default;

  ~CFG_production() = default;

  bool is_epsilon(const ALPHABET &alphabet) const;

  void print(std::ostream &os, const ALPHABET &alphabet) const;

private:
  grammar_symbol_type::nonterminal_type head;
  body_type body;
};

} // namespace cyy::computation
