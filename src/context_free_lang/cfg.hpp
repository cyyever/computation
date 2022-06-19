/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <cyy/algorithm/alphabet/alphabet.hpp>
#include <cyy/algorithm/alphabet/map_alphabet.hpp>

#include "cfg_production.hpp"
#include "formal_grammar/grammar_symbol.hpp"

namespace cyy::computation {

  class CFG {

  public:
    using terminal_type = grammar_symbol_type::terminal_type;
    using nonterminal_type = grammar_symbol_type::nonterminal_type;
    using terminal_set_type = std::unordered_set<terminal_type>;
    using nonterminal_set_type = std::unordered_set<nonterminal_type>;
    using production_body_set_type = std::set<CFG_production::body_type>;
    using production_set_type =
        std::unordered_map<nonterminal_type, production_body_set_type>;

    struct parse_node;
    using parse_node_ptr = std::shared_ptr<parse_node>;
    struct parse_node {
    public:
      explicit parse_node(grammar_symbol_type grammar_symbol_)
          : grammar_symbol(std::move(grammar_symbol_)) {}

      [[nodiscard]] std::string MMA_draw(const ALPHABET &alphabet) const;
      static parse_node_ptr
      make_parse_node(nonterminal_type head,
                      CFG_production::body_span_type body);
      grammar_symbol_type grammar_symbol;
      std::vector<parse_node_ptr> children;

    private:
      std::tuple<std::string, std::string, size_t>
      MMA_draw_edge(const ALPHABET &alphabet, size_t vertex_id) const;
    };

    CFG(ALPHABET_ptr alphabet_, nonterminal_type start_symbol_,
        production_set_type productions_);

    CFG(const CFG &) = default;
    CFG &operator=(const CFG &) = default;

    CFG(CFG &&) noexcept = default;
    CFG &operator=(CFG &&) noexcept = default;

    virtual ~CFG() = default;

    bool operator==(const CFG &rhs) const = default;

    bool has_production(const CFG_production &production) const;

    nonterminal_set_type get_heads() const;
    auto get_head_view() const {
      return productions | std::ranges::views::keys;
    }

    ALPHABET_ptr get_terminal_alphabet() const;
    std::shared_ptr<map_alphabet<std::string>> get_nonterminal_alphabet() const;
    ALPHABET_ptr get_full_alphabet() const;

    auto const &get_alphabet() const noexcept { return *alphabet; }
    auto get_alphabet_ptr() const noexcept { return alphabet; }

    auto const &get_productions() const &noexcept { return productions; }
    auto &get_productions() &&noexcept { return productions; }

    auto productions_view() const {
      return std::views::join(std::views::transform(productions, [](const auto &p) {
          return std::views::transform(p.second, [&p](auto const &t) {
              return std::pair<const CFG_production::head_type &,
              const CFG_production::body_type &>(p.first, t);
              });
          }));
    }

    const production_body_set_type &
    get_bodies(const nonterminal_type &head) const;

    terminal_set_type get_terminals() const;
    nonterminal_set_type get_nonterminals() const;

    bool has_left_recursion() const;
    void eliminate_left_recursion(std::vector<nonterminal_type> old_heads = {});

    const std::unordered_map<nonterminal_type,
                             std::pair<terminal_set_type, bool>> &
    first() const;

    std::pair<terminal_set_type, bool>
    first(const grammar_symbol_const_span_type &alpha) const;

    //! Convert grammar to Chomsky normal form
    void to_CNF();

    void left_factoring();

    bool recursive_descent_parse(symbol_string_view view) const;

    std::unordered_map<nonterminal_type, terminal_set_type> follow() const;

    const nonterminal_type &get_start_symbol() const noexcept {
      return start_symbol;
    }

    void remove_head(const nonterminal_type &head);

    bool contains(const grammar_symbol_type &grammar_symbol) const;
    [[nodiscard]] std::string MMA_draw() const;

    nonterminal_type get_new_head(nonterminal_type advise_head) const {
      do {
        advise_head.push_back('\'');
      } while (productions.contains(advise_head));
      return advise_head;
    }
    static void
    modify_body_set(production_body_set_type &body_set,
                    std::function<bool(CFG_production::body_type &)> fun);

  protected:
    void normalize_start_symbol();

    static nonterminal_type get_new_head(nonterminal_type advise_head,
                                         const nonterminal_set_type &heads) {
      do {
        advise_head.push_back('\'');
      } while (heads.contains(advise_head));
      return advise_head;
    }

    friend std::ostream &operator<<(std::ostream &os, const CFG &cfg);

  private:
    void normalize_productions();
    std::unordered_map<nonterminal_type, nonterminal_set_type>
    get_head_dependency() const;
    void eliminate_useless_symbols();
    void eliminate_epsilon_productions();
    void eliminate_single_productions();
    nonterminal_set_type nullable() const;

  protected:
    ALPHABET_ptr alphabet;

  private:
    nonterminal_type start_symbol;
    nonterminal_type old_start_symbol;
    production_set_type productions;
    mutable std::unordered_map<nonterminal_type,
                               std::pair<terminal_set_type, bool>>
        first_sets;
  };

} // namespace cyy::computation
