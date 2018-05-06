/*!
 * \file regex.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <cassert>

#include "common_tokens.hpp"
#include "exception.hpp"
#include "regex.hpp"
#include "slr_grammar.hpp"

namespace cyy::lang {

std::shared_ptr<regex::syntax_node>
regex::parse(symbol_string_view view) const {

  /*
     rexpr -> rexpr '|' rterm
     rexpr -> rterm

     rterm -> rterm rfactor
     rterm -> rfactor

     rfactor -> rfactor '*'
     rfactor -> rprimary

     rprimary -> '(' rexpr ')'
     rprimary -> '\' ASCII-char
     rprimary -> 'non-operator-char'

     ASCII-char -> 'any-ASCII-char'
  */

  std::set<CFG::terminal_type> operators{'|', '*', '(', '\\', ')'};

  std::map<CFG::nonterminal_type, std::vector<CFG::production_body_type>>
      productions;
  productions["rexpr"] = {
      {"rexpr", '|', "rterm"},
      {"rterm"},
  };
  productions["rterm"] = {
      {"rterm", "rfactor"},
      {"rfactor"},
  };
  productions["rfactor"] = {
      {"rfactor", '*'},
      {"rprimary"},
  };
  productions["rprimary"] = {
      {'(', "rexpr", ')'},
      {'\\', "ASCII-char"},
  };

  alphabet->foreach_symbol([&](auto const &a) {
    productions["ASCII-char"].emplace_back(CFG::production_body_type{a});

    if (!operators.count(a)) {
      productions["rprimary"].emplace_back(

          CFG::production_body_type{a});
    }
  });

  SLR_grammar regex_grammar("ASCII", "rexpr", productions);

  auto parse_tree = regex_grammar.parse(view);
  if (!parse_tree) {
    throw cyy::computation::exception::no_regular_expression("");
  }

  using syntax_node_ptr = std::shared_ptr<regex::syntax_node>;

  auto construct_syntex_tree =
      [](auto &&self,
         const CFG::parse_node_ptr &root_parse_node) -> syntax_node_ptr {
    std::shared_ptr<regex::syntax_node> root_syntax_node;

    if (auto ptr = std::get_if<CFG::nonterminal_type>(
            &(root_parse_node->grammar_symbol))) {
      if (*ptr == "rexpr") {
        if (root_parse_node->children.size() == 1) {
          return self(self, root_parse_node->children[0]);
        } else {
          return std::make_shared<regex::union_node>(
              self(self, root_parse_node->children[0]),
              self(self, root_parse_node->children[2]));
        }
      } else if (*ptr == "rterm") {
        if (root_parse_node->children.size() == 1) {
          return self(self, root_parse_node->children[0]);
        } else {
          return std::make_shared<regex::concat_node>(
              self(self, root_parse_node->children[0]),
              self(self, root_parse_node->children[1]));
        }
      } else if (*ptr == "rfactor") {
        if (root_parse_node->children.size() == 1) {
          return self(self, root_parse_node->children[0]);
        } else {
          return std::make_shared<regex::kleene_closure_node>(
              self(self, root_parse_node->children[0]));
        }
      } else if (*ptr == "rprimary") {
        auto first_terminal = std::get<CFG::terminal_type>(
            root_parse_node->children[0]->grammar_symbol);
        switch (first_terminal) {
        case '(':
          return self(self, root_parse_node->children[1]);
        case '\\':
          return self(self, root_parse_node->children[1]);
        default:
          return std::make_shared<regex::basic_node>(first_terminal);
        }
      } else if (*ptr == "ASCII-char") {
        auto first_terminal = std::get<CFG::terminal_type>(
            root_parse_node->children[0]->grammar_symbol);
        return std::make_shared<regex::basic_node>(first_terminal);
      }
    }
    assert(0);
    return {};
  };

  return construct_syntex_tree(construct_syntex_tree, parse_tree);
}
} // namespace cyy::lang
