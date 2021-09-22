
#include "common_grammar.hpp"

#include "lang/common_tokens.hpp"
namespace cyy::computation {

  std::shared_ptr<SLR_grammar> get_expression_grammar() {
    static std::shared_ptr<SLR_grammar> grammar;
    if (grammar) {
      return grammar;
    }

    CFG::production_set_type productions;
    auto id = static_cast<CFG::terminal_type>(common_token::id);
    productions["E"] = {
        {"E", U'+', "T"},
        {"T"},
    };
    productions["T"] = {
        {"T", U'*', "F"},
        {"F"},
        {'-', "F"},
    };
    productions["F"] = {{U'(', "E", U')'}, {id}};
    grammar = std::make_shared<SLR_grammar>("common_tokens", "E", productions);
    return grammar;
  }

} // namespace cyy::computation
