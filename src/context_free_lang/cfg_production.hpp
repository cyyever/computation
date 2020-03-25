/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <iosfwd>

#include "../formal_grammar/grammar_symbol.hpp"

namespace cyy::computation {

  class CFG_production final {

  public:
    using head_type = grammar_symbol_type::nonterminal_type;
    using body_type = grammar_symbol_string_type;
    using body_span_type = grammar_symbol_const_span_type;

    CFG_production(head_type head_, body_type body_)
        : head{std::move(head_)}, body{std::move(body_)} {}

    CFG_production(const CFG_production &) = default;
    CFG_production &operator=(const CFG_production &) = default;

    CFG_production(CFG_production &&) noexcept = default;
    CFG_production &operator=(CFG_production &&) noexcept = default;

    ~CFG_production() = default;

    std::strong_ordering operator<=>(const CFG_production &rhs) const {
      if (std::tie(head, body) < std::tie(rhs.head, rhs.body)) {
        return std::strong_ordering::less;
      } 
      if (std::tie(head, body) == std::tie(rhs.head, rhs.body)) {
        return std::strong_ordering::equal;
      }
      return std::strong_ordering::greater;
    }

    bool is_epsilon() const;

    void print(std::ostream &os, const ALPHABET &alphabet) const;

    auto get_head() const -> auto const & { return head; }
    auto get_body() const -> auto const & { return body; }

  private:
    void print_body(std::ostream &os, const ALPHABET &alphabet) const;

  private:
    grammar_symbol_type::nonterminal_type head;
    body_type body;
  };

} // namespace cyy::computation
