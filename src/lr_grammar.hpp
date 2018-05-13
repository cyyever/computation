/*!
 * \file lr_grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <functional>
#include <map>
#include <utility>

#include "cfg.hpp"

namespace cyy::computation {

class LR_grammar : public CFG {

public:
  using CFG::CFG;

  virtual ~LR_grammar() = default;

  parse_node_ptr parse(symbol_string_view view) const;

private:
  virtual void construct_parsing_table() = 0;

protected:
  std::map<std::pair<uint64_t, terminal_type>,
           std::variant<uint64_t, production_type, bool>>
      action_table;
  std::map<std::pair<uint64_t, nonterminal_type>, uint64_t> goto_table;
};
} // namespace cyy::computation
