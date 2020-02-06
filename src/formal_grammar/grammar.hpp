/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <gsl/span>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/span.hpp>
#include <range/v3/view/transform.hpp>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "../lang/alphabet.hpp"

namespace cyy::computation {
  class grammar_symbol_type : public std::variant<symbol_type, std::string> {

  public:
    using terminal_type = symbol_type;
    using nonterminal_type = std::string;
    using std::variant<symbol_type, std::string>::variant;
    grammar_symbol_type(char c)
        : grammar_symbol_type(static_cast<symbol_type>(c)) {}

    bool is_terminal() const noexcept {
      return std::holds_alternative<terminal_type>(*this);
    }
    bool is_nonterminal() const noexcept {
      return std::holds_alternative<nonterminal_type>(*this);
    }

    terminal_type get_terminal() const noexcept {
      return std::get<terminal_type>(*this);
    }

    const nonterminal_type *get_nonterminal_ptr() const noexcept {
      return std::get_if<nonterminal_type>(this);
    }

    const nonterminal_type &get_nonterminal() const noexcept {
      return *std::get_if<nonterminal_type>(this);
    }
    bool operator==(const terminal_type &t) const {
      return is_terminal() && get_terminal() == t;
    }
    bool operator!=(const terminal_type &t) const { return !operator==(t); }
    bool operator==(const nonterminal_type &t) const {
      return is_nonterminal() && *get_nonterminal_ptr() == t;
    }
    bool operator!=(const nonterminal_type &t) const { return !operator==(t); }

    void print(std::ostream &os, const ALPHABET &alphabet) const {
      if (is_terminal()) {
        alphabet.print(os, get_terminal());
      } else {
        os << *get_nonterminal_ptr();
      }
    }
  };

  class grammar_symbol_string_type : public std::vector<grammar_symbol_type> {
  public:
    using std::vector<grammar_symbol_type>::vector;
    auto get_terminal_view() const -> auto {
      return *this |
             ranges::views::filter([](auto g) { return g.is_terminal(); }) |
             ranges::views::transform([](auto g) { return g.get_terminal(); });
    }
    auto get_nonterminal_view() const -> auto {
      return *this |
             ranges::views::filter([](auto g) { return g.is_nonterminal(); }) |
             ranges::views::transform(
                 [](auto g) { return *g.get_nonterminal_ptr(); });
    }
  };

  using grammar_symbol_const_span_type = gsl::span<const grammar_symbol_type>;
} // namespace cyy::computation
