/*!
 * \file regex.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <map>
#include <set>

#include "../contex_free_lang/lr_grammar.hpp"
#include "dfa.hpp"
#include "nfa.hpp"

namespace cyy::computation {

class regex {

public:
  class syntax_node {
  public:
    virtual ~syntax_node() = default;
    virtual NFA to_NFA(const ALPHABET &alphabet,
                       uint64_t start_state) const = 0;
    virtual bool nullable() const = 0;
    virtual void
    assign_position(std::map<uint64_t, symbol_type> &position_to_symbol) = 0;
    virtual std::set<uint64_t> first_pos() const = 0;
    virtual std::set<uint64_t> last_pos() const = 0;
    virtual std::map<uint64_t, std::set<uint64_t>> follow_pos() const = 0;
  };

  class epsilon_node : public syntax_node {
  public:
    explicit epsilon_node() {}
    NFA to_NFA(const ALPHABET &alphabet, uint64_t start_state) const override;
    bool nullable() const override { return true; }
    void assign_position(
        std::map<uint64_t, symbol_type> &position_to_symbol) override;
    std::set<uint64_t> first_pos() const override;
    std::set<uint64_t> last_pos() const override;
    std::map<uint64_t, std::set<uint64_t>> follow_pos() const override {
      return {};
    }
  };

  class basic_node : public syntax_node {
  public:
    explicit basic_node(symbol_type symbol_) : symbol(symbol_) {}
    NFA to_NFA(const ALPHABET &alphabet, uint64_t start_state) const override;
    bool nullable() const override { return false; }
    void assign_position(
        std::map<uint64_t, symbol_type> &position_to_symbol) override;
    std::set<uint64_t> first_pos() const override;
    std::set<uint64_t> last_pos() const override;
    std::map<uint64_t, std::set<uint64_t>> follow_pos() const override {
      return {};
    }

  private:
    symbol_type symbol;
    uint64_t position{0};
  };
  class union_node : public syntax_node {
  public:
    explicit union_node(const std::shared_ptr<syntax_node> &left_node_,
                        const std::shared_ptr<syntax_node> &right_node_)
        : left_node(left_node_), right_node(right_node_) {}
    NFA to_NFA(const ALPHABET &alphabet, uint64_t start_state) const override;
    bool nullable() const override {
      return left_node->nullable() || right_node->nullable();
    }
    void assign_position(
        std::map<uint64_t, symbol_type> &position_to_symbol) override;
    std::set<uint64_t> first_pos() const override;
    std::set<uint64_t> last_pos() const override;
    std::map<uint64_t, std::set<uint64_t>> follow_pos() const override;

  private:
    std::shared_ptr<syntax_node> left_node, right_node;
  };
  class concat_node : public syntax_node {
  public:
    explicit concat_node(const std::shared_ptr<syntax_node> &left_node_,
                         const std::shared_ptr<syntax_node> &right_node_)
        : left_node(left_node_), right_node(right_node_) {}
    NFA to_NFA(const ALPHABET &alphabet, uint64_t start_state) const override;
    bool nullable() const override {
      return left_node->nullable() && right_node->nullable();
    }
    void assign_position(
        std::map<uint64_t, symbol_type> &position_to_symbol) override;
    std::set<uint64_t> first_pos() const override;
    std::set<uint64_t> last_pos() const override;
    std::map<uint64_t, std::set<uint64_t>> follow_pos() const override;

  private:
    std::shared_ptr<syntax_node> left_node, right_node;
  };
  class kleene_closure_node : public syntax_node {
  public:
    explicit kleene_closure_node(
        const std::shared_ptr<syntax_node> &inner_node_)
        : inner_node(inner_node_) {}
    NFA to_NFA(const ALPHABET &alphabet, uint64_t start_state) const override;
    bool nullable() const override { return true; }
    void assign_position(
        std::map<uint64_t, symbol_type> &position_to_symbol) override;
    std::set<uint64_t> first_pos() const override;
    std::set<uint64_t> last_pos() const override;

  private:
    std::shared_ptr<syntax_node> inner_node;
    std::map<uint64_t, std::set<uint64_t>> follow_pos() const override;
  };

public:
  regex(const std::string &alphabet_name, symbol_string_view view)
      : alphabet(ALPHABET::get(alphabet_name)) {
    syntax_tree = parse(view);
  }

  NFA to_NFA(uint64_t start_state = 0) const {
    return syntax_tree->to_NFA(*alphabet, start_state);
  }

  //基于McNaughton-Yamada算法
  DFA to_DFA() const;

private:
  std::shared_ptr<syntax_node> parse(symbol_string_view view) const;
  std::shared_ptr<syntax_node>
  make_character_class(const std::set<symbol_type> &symbol_set) const;
  std::shared_ptr<syntax_node> make_complemented_character_class(
      const std::set<symbol_type> &symbol_set) const;

  static std::shared_ptr<LR_grammar> get_grammar();
  static inline std::shared_ptr<ALPHABET> regex_alphabet{
      ALPHABET::get("printable-ASCII")};

private:
  std::shared_ptr<ALPHABET> alphabet;
  mutable std::shared_ptr<regex::syntax_node> syntax_tree;
};
} // namespace cyy::computation
