/*!
 * \file grammar_symbol.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <alphabet/symbol.hpp>
#include <cyy/algorithm/alphabet/alphabet.hpp>
#include <cyy/algorithm/hash.hpp>
import std;

namespace cyy::computation {
  struct grammar_symbol_type
      : public std::variant<cyy::algorithm::symbol_type, std::string> {

    using terminal_type = cyy::algorithm::symbol_type;
    using nonterminal_type = std::string;
    using ALPHABET = cyy::algorithm::ALPHABET;
    using std::variant<terminal_type, nonterminal_type>::variant;
    grammar_symbol_type(char c) noexcept
        : grammar_symbol_type(static_cast<terminal_type>(c)) {}

    bool is_terminal() const noexcept {
      return std::holds_alternative<terminal_type>(*this);
    }
    bool is_nonterminal() const noexcept {
      return std::holds_alternative<nonterminal_type>(*this);
    }

    terminal_type get_terminal() const {
      return std::get<terminal_type>(*this);
    }
    auto get_terminal_ptr() const noexcept {
      return std::get_if<terminal_type>(this);
    }

    const nonterminal_type *get_nonterminal_ptr() const noexcept {
      return std::get_if<nonterminal_type>(this);
    }

    const nonterminal_type &get_nonterminal() const {
      return std::get<nonterminal_type>(*this);
    }
    bool operator==(const terminal_type &t) const noexcept {
      auto const *ptr = get_terminal_ptr();
      return ptr && *ptr == t;
    }
    bool operator==(const nonterminal_type &t) const noexcept {
      auto const *ptr = get_nonterminal_ptr();
      return ptr && *ptr == t;
    }
    std::string to_string(const ALPHABET &alphabet) const {
      if (is_terminal()) {
        return alphabet.to_string(get_terminal());
      }
      return *get_nonterminal_ptr();
    }
    std::string MMA_draw(const ALPHABET &alphabet) const {
      if (is_terminal()) {
        return alphabet.MMA_draw(get_terminal());
      }
      return std::string("Style[\"") + get_nonterminal() + "\", Bold, Italic]";
    }
  };

  struct grammar_symbol_string_type : public std::vector<grammar_symbol_type> {
    using std::vector<grammar_symbol_type>::vector;
    auto get_terminal_view() const noexcept {
      return *this | std::ranges::views::filter([](const auto &g) noexcept {
        return g.is_terminal();
      }) | std::ranges::views::transform([](const auto &g) {
        return g.get_terminal();
      });
    }
    auto get_nonterminal_view() const {
      return *this | std::ranges::views::filter([](const auto &g) {
        return g.is_nonterminal();
      }) | std::ranges::views::transform([](const auto &g) {
        return *g.get_nonterminal_ptr();
      });
    }
  };
  using grammar_symbol_const_span_type = std::span<const grammar_symbol_type>;
} // namespace cyy::computation

namespace std {
  template <>
  struct hash<cyy::computation::grammar_symbol_type>
      : public std::hash<std::variant<
            cyy::computation::grammar_symbol_type::terminal_type,
            cyy::computation::grammar_symbol_type::nonterminal_type>> {};
} // namespace std
