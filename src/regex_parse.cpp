/*!
 * \file regex.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "common_tokens.hpp"
#include "grammar.hpp"
#include "regex.hpp"

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

     =>

     rexpr -> rterm rexpr'
     rexpr' -> '|' rterm rexpr'
     rexpr' -> epsilon

     rterm -> rfactor rterm'
     rterm' -> rfactor rterm'
     rterm' -> epsilon

     rfactor -> rprimary rfactor'
     rfactor' -> '*' rfactor'
     rfactor' -> epsilon

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

  CFG cfg("ASCII", "rexpr", productions);
  cfg.eliminate_left_recursion({"rexpr", "rterm", "rfactor", "rprimary"});
  LL_grammar regex_grammar(std::move(cfg));

  //LL_grammar regex_grammar("ASCII", "rexpr", productions);

  auto parse_tree = regex_grammar.parse(view);
  if (!parse_tree) {
    throw std::runtime_error("regex grammar is not LL(1) grammar");
  }

  using syntax_node_ptr = std::shared_ptr<regex::syntax_node>;

  auto construct_syntex_tree =
      [](auto &&self, const CFG::parse_node_ptr &root_parse_node,
         syntax_node_ptr left_node) -> syntax_node_ptr {
    std::shared_ptr<regex::syntax_node> root_syntax_node;

    if (auto ptr = std::get_if<CFG::nonterminal_type>(
            &(root_parse_node->grammar_symbol))) {
      if (*ptr == "rexpr") {
        left_node = self(self, root_parse_node->children[0], nullptr);
        return self(self, root_parse_node->children[1], left_node);
      }
      if (*ptr == "rexpr'") {
        if (root_parse_node->children.size() ==
            3) { //  rexpr' -> '|' rterm rexpr'
          return self(self, root_parse_node->children[2],
                      std::make_shared<regex::union_node>(
                          left_node,
                          self(self, root_parse_node->children[1], nullptr)));
        }
        return left_node;
      }
      if (*ptr == "rterm") {
        left_node = self(self, root_parse_node->children[0], nullptr);
        return self(self, root_parse_node->children[1], left_node);
      }
      if (*ptr == "rterm'") {
        if (root_parse_node->children.size() == 2) { // rterm' -> rfactor rterm'
          return self(self, root_parse_node->children[1],
                      std::make_shared<regex::concat_node>(
                          left_node,
                          self(self, root_parse_node->children[0], nullptr)));
        }
        return left_node;

      } else if (*ptr == "rfactor") {
        left_node = self(self, root_parse_node->children[0], nullptr);
        return self(self, root_parse_node->children[1], left_node);
      } else if (*ptr == "rfactor'") {
        if (root_parse_node->children.size() == 2) { // rfactor' -> '*' rfactor'
          return self(self, root_parse_node->children[1],
                      std::make_shared<regex::kleene_closure_node>(left_node));
        }
        return left_node;

      } else if (*ptr == "rprimary") {
        auto first_terminal = std::get<CFG::terminal_type>(
            root_parse_node->children[0]->grammar_symbol);
        switch (first_terminal) {
        case '(':
          return self(self, root_parse_node->children[1], nullptr);
        case '\\':
          return self(self, root_parse_node->children[1], nullptr);
        default:
          return std::make_shared<regex::basic_node>(first_terminal);
        }
      } else if (*ptr == "ASCII-char") {
        auto first_terminal = std::get<CFG::terminal_type>(
            root_parse_node->children[0]->grammar_symbol);
        return std::make_shared<regex::basic_node>(first_terminal);
      }
    }
    throw std::runtime_error("should no go here");
  };

  return construct_syntex_tree(construct_syntex_tree, parse_tree, nullptr);
}
} // namespace cyy::lang
