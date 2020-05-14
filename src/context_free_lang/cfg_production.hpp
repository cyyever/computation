/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <iosfwd>

#include "../formal_grammar/grammar_symbol.hpp"
#include "../hash.hpp"

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

    bool operator==(const CFG_production &rhs) const {
      return std::tie(head, body) == std::tie(rhs.head, rhs.body);
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
namespace std {
  template <> struct hash<cyy::computation::CFG_production> {
    std::size_t
    operator()(const cyy::computation::CFG_production &x) const noexcept {
      size_t seed = 0;

      auto const &head = x.get_head();
      auto const &body = x.get_body();
      boost::hash_combine(
          seed, std::hash<cyy::computation::CFG_production::head_type>()(head));
      boost::hash_combine(
          seed, std::hash<cyy::computation::CFG_production::body_type>()(body));
      return seed;
    }
  };
} // namespace std
