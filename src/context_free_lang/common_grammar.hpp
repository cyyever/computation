
#pragma once

#include <memory>

#include "context_free_lang/slr_grammar.hpp"

namespace cyy::computation {
  std::shared_ptr<SLR_grammar> get_expression_grammar();

}
