
#pragma once


#include "context_free_lang/slr_grammar.hpp"

namespace cyy::computation {
  CFG::production_set_type get_expression_productions();
  std::shared_ptr<SLR_grammar> get_expression_grammar();

} // namespace cyy::computation
