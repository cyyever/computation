/*!
 * \file lalr_grammar.hpp
 *
 * \author cyy
 * \date 2018-09-24
 */

#pragma once

#include <unordered_map>

#include "lr_1_grammar.hpp"

namespace cyy::computation {

  class LALR_grammar final : public LR_1_grammar {

  public:
    using LR_1_grammar::LR_1_grammar;

    std::pair<collection_type, goto_table_type> get_collection() const override;

  private:
    using lookahead_map_type = std::unordered_map<
        grammar_symbol_type,
        std::unordered_map<CFG_production, std::pair<bool, terminal_set_type>>>;
    lookahead_map_type check_lookahead(const LR_0_item &item) const;
  };
} // namespace cyy::computation
