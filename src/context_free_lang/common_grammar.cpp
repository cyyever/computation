
#include "common_grammar.hpp"

#include "alphabet/common_tokens.hpp"
namespace cyy::computation {

  CFG::production_set_type get_expression_productions() {
    CFG::production_set_type productions;
    if (!productions.empty()) {
      return productions;
    }
    auto id = static_cast<CFG::terminal_type>(cyy::algorithm::common_token::id);
    auto number =
        static_cast<CFG::terminal_type>(cyy::algorithm::common_token::number);
    productions["Logical"] = {
        {"Logical", U'&', U'&', "E"},
        {"Logical", U'|', U'|', "E"},
        {"E"},
    };
    productions["E"] = {
        {"E", U'+', "T"},
        {"E", U'-', "T"},
        {"T"},
    };
    productions["T"] = {
        {"T", U'*', "F"}, {"T", U'/', "F"}, {"F"}, {'-', "F"}, {'!', "F"},
    };
    productions["F"] = {{U'(', "E", U')'}, {id}, {"array"}, {number}};
    productions["array"] = {{id, '[', "E", ']'}, {"array", '[', "E", ']'}};
    return productions;
  }
  std::shared_ptr<SLR_grammar> get_expression_grammar() {
    static std::shared_ptr<SLR_grammar> const grammar =
        std::make_shared<SLR_grammar>("common_tokens", "E",
                                      get_expression_productions());
    return grammar;
  }

} // namespace cyy::computation
