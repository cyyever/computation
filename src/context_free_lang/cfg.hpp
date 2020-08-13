/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "cfg_production.hpp"
#include "formal_grammar/grammar_symbol.hpp"
#include "lang/alphabet.hpp"
#include "lr_0_item.hpp"
#include "regular_lang/dfa.hpp"

namespace cyy::computation {

  class CFG {

  public:
    using terminal_type = grammar_symbol_type::terminal_type;
    using nonterminal_type = grammar_symbol_type::nonterminal_type;
    using production_set_type =
        std::map<nonterminal_type, std::vector<CFG_production::body_type>>;

    struct parse_node;
    using parse_node_ptr = std::shared_ptr<parse_node>;
    struct parse_node {
    public:
      explicit parse_node(grammar_symbol_type grammar_symbol_)
          : grammar_symbol(std::move(grammar_symbol_)) {}

      std::string MMA_draw(const ALPHABET&alphabet) const;
      static parse_node_ptr
      make_parse_node(nonterminal_type head,
                      CFG_production::body_span_type body);
      grammar_symbol_type grammar_symbol;
      std::vector<parse_node_ptr> children;

    private:
      std::pair<std::string,size_t>MMA_draw_edge(const ALPHABET&alphabet,size_t vertex_id) const;
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

    std::set<nonterminal_type> get_heads() const;

    auto const &get_alphabet() const noexcept { return *alphabet; }

    auto const &get_productions() const noexcept { return productions; }
    const std::vector<CFG_production::body_type> &
    get_bodies(const nonterminal_type &head) const;

    std::set<terminal_type> get_terminals() const;
    std::set<nonterminal_type> get_nonterminals() const;

    void eliminate_useless_symbols();

    bool has_left_recursion() const;
    void eliminate_left_recursion(std::vector<nonterminal_type> old_heads = {});

    void eliminate_epsilon_productions();

    void eliminate_single_productions();

    //! convert grammar to Chomsky normal form
    void to_CNF();

    void left_factoring();

    bool recursive_descent_parse(symbol_string_view view) const;

    const std::map<nonterminal_type, std::pair<std::set<terminal_type>, bool>> &
    first() const;

    std::map<nonterminal_type, std::set<terminal_type>> follow() const;

    std::unordered_set<nonterminal_type> nullable() const;

    std::pair<std::set<terminal_type>, bool>
    first(const grammar_symbol_const_span_type &alpha) const;

    const nonterminal_type &get_start_symbol() const noexcept {
      return start_symbol;
    }

    using lr_0_item_set_collection_type =
        std::unordered_map<DFA::state_type, new_LR_0_item_set>;

    std::tuple<DFA, std::unordered_map<nonterminal_type, symbol_type>,
               std::unordered_map<symbol_type, nonterminal_type>,
               lr_0_item_set_collection_type>
    get_DK() const;
    void normalize_start_symbol();

    void remove_head(nonterminal_type head,
                     nonterminal_type new_start_symbol = {});

    bool contains(const grammar_symbol_type &grammar_symbol) const;
    std::string MMA_draw() const;

  protected:
    nonterminal_type get_new_head(nonterminal_type advise_head) const {
      do {
        advise_head.push_back('\'');
      } while (productions.contains(advise_head));
      return advise_head;
    }

    void normalize_productions();
    static nonterminal_type
    get_new_head(nonterminal_type advise_head,
                 const std::set<nonterminal_type> &heads) {
      do {
        advise_head.push_back('\'');
      } while (heads.contains(advise_head));
      return advise_head;
    }

    friend std::ostream &operator<<(std::ostream &os, const CFG &cfg);

  private:
    std::map<nonterminal_type, std::set<nonterminal_type>>
    get_head_dependency() const;

  protected:
    ALPHABET_ptr alphabet;
    nonterminal_type start_symbol;
    nonterminal_type old_start_symbol;
    production_set_type productions;

  private:
    mutable std::map<nonterminal_type, std::pair<std::set<terminal_type>, bool>>
        first_sets;
  };

} // namespace cyy::computation
