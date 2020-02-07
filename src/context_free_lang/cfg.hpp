/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "../formal_grammar/grammar.hpp"
#include "../lang/alphabet.hpp"
#include "cfg_production.hpp"
#include "pda.hpp"

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
      explicit parse_node(grammar_symbol_type grammar_symbol_)
          : grammar_symbol(std::move(grammar_symbol_)) {}

      grammar_symbol_type grammar_symbol;
      std::vector<parse_node_ptr> children;

      static parse_node_ptr
      make_parse_node(nonterminal_type head,
                      CFG_production::body_span_type body);
    };

    CFG(const std::string &alphabet_name, nonterminal_type start_symbol_,
        production_set_type productions_);

    CFG(const CFG &) = default;
    CFG &operator=(const CFG &) = default;

    CFG(CFG &&) noexcept = default;
    CFG &operator=(CFG &&) noexcept = default;

    virtual ~CFG() = default;

    bool operator==(const CFG &rhs) const;

    bool has_production(const CFG_production &production) const;

    std::set<nonterminal_type> get_heads() const;

    auto get_alphabet() const noexcept -> const auto & { return *alphabet; }

    auto get_productions() const noexcept -> const auto & {
      return productions;
    }

    std::set<terminal_type> get_terminals() const;

    void eliminate_useless_symbols();

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

    bool is_CNF() const;

    std::set<nonterminal_type> nullable() const;

    std::pair<std::set<terminal_type>, bool>
    first(const grammar_symbol_const_span_type &alpha) const;

    const nonterminal_type &get_start_symbol() const { return start_symbol; }

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

  protected:
    std::shared_ptr<ALPHABET> alphabet;
    nonterminal_type start_symbol;
    std::map<nonterminal_type, std::vector<CFG_production::body_type>>
        productions;

  private:
    mutable std::map<nonterminal_type, std::pair<std::set<terminal_type>, bool>>
        first_sets;
  };

} // namespace cyy::computation
