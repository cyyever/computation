/*!
 * \file ll_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <functional>

#include "cfg.hpp"
#include "hash.hpp"

namespace cyy::computation {

  class LL_grammar final : public CFG {
  public:
    using CFG::CFG;

    [[nodiscard]] bool
    parse(symbol_string_view view,
          const std::function<void(const CFG_production &, size_t pos)>
              &match_callback) const;

    parse_node_ptr get_parse_tree(symbol_string_view view) const;

  private:
    void construct_parsing_table() const;

  private:
    mutable std::unordered_map<
        std::pair<CFG::terminal_type, CFG::nonterminal_type>,
        const CFG_production::body_type &>
        parsing_table;
  };
} // namespace cyy::computation
