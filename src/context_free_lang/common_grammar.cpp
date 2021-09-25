
#include "common_grammar.hpp"

#include "lang/common_tokens.hpp"
namespace cyy::computation {

  CFG::production_set_type get_expression_productions() {
    CFG::production_set_type productions;
    if (!productions.empty()) {
      return productions;
    }
    auto id = static_cast<CFG::terminal_type>(common_token::id);
    productions["L"] = {
        {"L", U'&', U'&', "E"},
        {"L", U'|', U'|', "E"},
        {"E"},
    };
    productions["E"] = {
        {"E", U'+', "T"},
        {"E", U'-', "T"},
        {"T"},
    };
    productions["T"] = {
        {"T", U'*', "F"},
        {"T", U'/', "F"},
        {"F"},
        {'-', "F"},
    };
    productions["F"] = {{U'(', "E", U')'}, {id}};
    return productions;
  }
  std::shared_ptr<SLR_grammar> get_expression_grammar() {
    static std::shared_ptr<SLR_grammar> grammar = std::make_shared<SLR_grammar>(
        "common_tokens", "E", get_expression_productions());
    return grammar;
  }

} // namespace cyy::computation
