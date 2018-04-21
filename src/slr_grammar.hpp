/*!
 * \file slr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <functional>
#include <set>
#include <unordered_set>

#include "grammar.hpp"
#include "lang.hpp"
#include "lr_item.hpp"

/*
namespace cyy::lang {

  struct LR_0_item {
    CFG::nonterminal_type head;
    CFG::production_body_type body;
    size_t dot_pos;
     bool operator ==(const  LR_0_item &rhs) const {
       return head==rhs.head && body==rhs.body && dot_pos==rhs.dot_pos;
     }
  };

}

namespace std {
  template <> struct hash<cyy::lang::LR_0_item>
  {
    size_t operator()(const cyy::lang::LR_0_item & x) const
    {
      auto hash_value=::std::hash<cyy::lang::CFG::nonterminal_type>()(x.head) ^

  ::std::hash<decltype(x.body.size())>()(x.body.size())	 ^
        ::std::hash<decltype(x.dot_pos)>()(x.dot_pos);
      return hash_value;
    }
  };
}
*/

namespace cyy::lang {

class SLR_grammar : public CFG {

public:
  /*
  struct LR_0_item_set {
    std::set<nonterminal_type> nonkernel_items   ;
    std::unordered_set<LR_0_item> kernel_items;

  };
  */

  SLR_grammar(const std::string &alphabet_name,
              const nonterminal_type &start_symbol_,
              std::map<nonterminal_type, std::vector<production_body_type>>
                  &productions_)
      : CFG(alphabet_name, start_symbol_, productions_) {
    /*
    if(!is_LL1()) {
      throw std::invalid_argument("not a LL1 grammar");
    }
    */
  }

  // parse_node_ptr parse(symbol_string_view view) const;

private:
  LR_0_item_set closure(LR_0_item_set set) const;
  LR_0_item_set GOTO(LR_0_item_set set,
                     const grammar_symbol_type &symbol) const;

  /*
  bool is_LL1() const;

  bool is_LL1(const std::map<nonterminal_type, std::set<terminal_type>>
                  &nonterminal_first_sets,
              const std::map<nonterminal_type, std::set<terminal_type>>
                  &follow_sets) const;

                  */
};
} // namespace cyy::lang
