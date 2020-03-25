/*!
 * \file lalr_grammar.hpp
 *
 * \author cyy
 * \date 2018-09-24
 */

#pragma once

#include <functional>
#include <set>
#include <unordered_map>

#include "canonical_lr_grammar.hpp"
#include "lr_item.hpp"

namespace cyy::computation {

  class LALR_grammar final : public canonical_LR_grammar {

  public:
    using canonical_LR_grammar::canonical_LR_grammar;

    std::pair<collection_type, goto_transition_set_type>
    canonical_collection() const override;

  private:
    using lookahead_map_type = std::unordered_map<
        grammar_symbol_type,
        std::unordered_map<CFG_production, std::pair<bool, std::set<terminal_type>>>>;
    lookahead_map_type check_lookahead(const LR_0_item &item) const;
    void construct_parsing_table() const override;
  };
} // namespace cyy::computation
