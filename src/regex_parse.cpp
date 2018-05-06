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
     rfactor -> rfactor '+'
     rfactor -> rfactor '?'
     rfactor -> rprimary

     rprimary -> '(' rexpr ')'
     rprimary -> escape-sequence
     rprimary -> 'non-operator-char'

     escape-sequence -> '\' 'any-char'
  */

  std::set<CFG::terminal_type> operators{'|', '*', '(', '\\', ')', '+', '?'};

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
      {"rfactor", '+'},
      {"rfactor", '?'},
      {"rprimary"},
  };
  productions["rprimary"] = {
      {'(', "rexpr", ')'},
      {"escape-sequence"},
  };

  alphabet->foreach_symbol([&](auto const &a) {
    productions["escape-sequence"].emplace_back(CFG::production_body_type{'\\',  a});

    if (!operators.count(a)) {
      productions["rprimary"].emplace_back(
          CFG::production_body_type{a});
    }
  });

  //SLR_grammar regex_grammar(alphabet->name(), "rexpr", productions);
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
        }
        return std::make_shared<regex::union_node>(
            self(self, root_parse_node->children[0]),
            self(self, root_parse_node->children[2]));
      } else if (*ptr == "rterm") {
        if (root_parse_node->children.size() == 1) {
          return self(self, root_parse_node->children[0]);
        }
        return std::make_shared<regex::concat_node>(
            self(self, root_parse_node->children[0]),
            self(self, root_parse_node->children[1]));
      } else if (*ptr == "rfactor") {
        if (root_parse_node->children.size() == 1) {
          return self(self, root_parse_node->children[0]);
        }

        auto second_terminal = std::get<CFG::terminal_type>(
            root_parse_node->children[1]->grammar_symbol);

        auto inner_tree = self(self, root_parse_node->children[0]);
        if (second_terminal == '*') {
          return std::make_shared<regex::kleene_closure_node>(inner_tree);
        } else if (second_terminal == '+') {
          return std::make_shared<regex::concat_node>(
              inner_tree,
              std::make_shared<regex::kleene_closure_node>(inner_tree));
        } else if (second_terminal == '?') {
          return std::make_shared<regex::union_node>(
              std::make_shared<regex::epsilon_node>(), inner_tree);
        }
      } else if (*ptr == "rprimary") {
        if ( std::holds_alternative<CFG::nonterminal_type>(root_parse_node->children[0]->grammar_symbol)) {
          return self(self, root_parse_node->children[0]);
        }

        auto first_terminal = std::get<CFG::terminal_type>(
            root_parse_node->children[0]->grammar_symbol);
        switch (first_terminal) {
        case '(':
          return self(self, root_parse_node->children[1]);
        default:
          return std::make_shared<regex::basic_node>(first_terminal);
        }
      } else if (*ptr == "escape-sequence") {
        auto second_terminal = std::get<CFG::terminal_type>(
            root_parse_node->children[1]->grammar_symbol);

	switch(second_terminal) {
		case 'f':
        return std::make_shared<regex::basic_node>('\f');
		case 'n':
        return std::make_shared<regex::basic_node>('\n');
		case 'r':
        return std::make_shared<regex::basic_node>('\r');
		case 't':
        return std::make_shared<regex::basic_node>('\t');
		case 'v':
        return std::make_shared<regex::basic_node>('\v');




		default:

        return std::make_shared<regex::basic_node>(second_terminal);
	}
      std::cout<<"*ptr="<<*ptr<<std::endl;
      }


    }
    assert(0);
    return {};
  };

  return construct_syntex_tree(construct_syntex_tree, parse_tree);
}
} // namespace cyy::lang
