/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include "../formal_grammar/grammar_symbol.hpp"

namespace cyy::computation {

  class CFG_production final {

  public:
    using ALPHABET = cyy::algorithm::ALPHABET;
    using head_type = grammar_symbol_type::nonterminal_type;
    using body_type = grammar_symbol_string_type;
    using body_span_type = grammar_symbol_const_span_type;

    CFG_production(head_type head_, body_type body_) noexcept
        : head{std::move(head_)}, body{std::move(body_)} {}

    CFG_production(const CFG_production &) = default;
    CFG_production &operator=(const CFG_production &) = default;

    CFG_production(CFG_production &&) noexcept = default;
    CFG_production &operator=(CFG_production &&) noexcept = default;

    ~CFG_production() = default;

    bool operator==(const CFG_production &rhs) const noexcept = default;
    // auto operator<=>(const CFG_production &rhs) const noexcept = default;
    bool is_epsilon() const noexcept;

    std::string to_string(const ALPHABET &alphabet) const;

    auto const &get_head() const noexcept { return head; }
    auto const &get_body() const noexcept { return body; }

    [[nodiscard]] std::string MMA_draw(
        const ALPHABET &alphabet, bool emphasize_head = false,
        const std::function<std::string(size_t)> &pos_callback = [](size_t) {
          return "";
        }) const;

  private:
    std::string body_to_string(const ALPHABET &alphabet) const;
    grammar_symbol_type::nonterminal_type head;
    body_type body;
  };

} // namespace cyy::computation
namespace std {
  template <> struct hash<cyy::computation::CFG_production> {
    std::size_t operator()(const cyy::computation::CFG_production &x) const {
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
