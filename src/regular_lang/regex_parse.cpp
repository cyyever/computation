/*!
 * \file regex.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <cassert>

#include "../contex_free_lang/slr_grammar.hpp"
#include "../exception.hpp"
#include "regex.hpp"

namespace cyy::computation {

/*
   rexpr -> rexpr '|' rterm
   rexpr -> rterm

   rterm -> rterm rfactor
   rterm -> rfactor

   rfactor -> rfactor '*'
   rfactor -> rfactor '+'
   rfactor -> rfactor '?'
   rfactor -> rprimary

   rprimary -> 'non-operator-char'
   rprimary -> escape-sequence
   rprimary -> '(' rexpr ')'
   rprimary -> '[' character-class ']'

   character-class -> 'any-char-except-backslash-and-]' character-class
   character-class -> escape-sequence character-class
   character-class -> epsilon

   escape-sequence -> '\' 'any-char'
*/
std::shared_ptr<LR_grammar> regex::get_grammar() {
  static std::shared_ptr<LR_grammar> regex_grammar;
  if (regex_grammar) {
    return regex_grammar;
  }

  std::set<CFG::terminal_type> operators{'|', '*', '(', '\\', ')',
                                         '+', '?', '[', ']'};

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
      {"escape-sequence"},
      {'(', "rexpr", ')'},
      {'[', "character-class", ']'},
  };

  productions["character-class"] = {{"escape-sequence", "character-class"},
                                    {regex_alphabet->get_epsilon()}};

  regex_alphabet->foreach_symbol([&](auto const &a) {
    productions["escape-sequence"].emplace_back(
        CFG::production_body_type{'\\', a});

    if (!operators.count(a)) {
      productions["rprimary"].emplace_back(CFG::production_body_type{a});
    }

    if (a != '\\' && a != ']') {
      productions["character-class"].emplace_back(
          CFG::production_body_type{a, "character-class"});
    }
  });

  regex_grammar = std::make_shared<SLR_grammar>(regex_alphabet->name(), "rexpr",
                                                productions);

  return regex_grammar;
}

std::shared_ptr<regex::syntax_node>
regex::make_character_class(const std::set<symbol_type> &symbol_set) const {

  std::shared_ptr<regex::syntax_node> root{};
  for (auto const symbol : symbol_set) {
    if (!root) {
      root = std::make_shared<regex::basic_node>(symbol);
    } else {
      root = std::make_shared<regex::union_node>(
          root, std::make_shared<regex::basic_node>(symbol));
    }
  }

  if (!root) {
    assert(0);
  }
  return root;
}

std::shared_ptr<regex::syntax_node> regex::make_complemented_character_class(
    const std::set<symbol_type> &symbol_set) const {
  std::shared_ptr<regex::syntax_node> root{};

  std::set<symbol_type> complemented_symbol_set;

  regex_alphabet->foreach_symbol([&](auto const &a) {
    if (!symbol_set.count(a)) {
      complemented_symbol_set.insert(a);
    }
  });

  return make_character_class(complemented_symbol_set);
}

std::shared_ptr<regex::syntax_node>
regex::parse(symbol_string_view view) const {

  auto parse_tree = get_grammar()->parse(view);
  if (!parse_tree) {
    throw cyy::computation::exception::no_regular_expression("");
  }

  using syntax_node_ptr = std::shared_ptr<regex::syntax_node>;

  auto parse_escape_sequence =
      [](const CFG::parse_node_ptr &parse_node) -> symbol_type {
    auto second_terminal =
        *(parse_node->children[1]->grammar_symbol.get_terminal_ptr());

    switch (second_terminal) {
    case 'f':
      return '\f';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
    case 'v':
      return '\v';
    default:
      return second_terminal;
    }
  };

  auto construct_syntax_tree =
      [this, &parse_escape_sequence](
          auto &&self,
          const CFG::parse_node_ptr &root_parse_node) -> syntax_node_ptr {
    std::shared_ptr<regex::syntax_node> root_syntax_node;

    if (auto ptr = root_parse_node->grammar_symbol.get_nonterminal_ptr()) {
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

        auto second_terminal =
            *(root_parse_node->children[1]->grammar_symbol.get_terminal_ptr());

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
        if (root_parse_node->children[0]->grammar_symbol.is_nonterminal()) {
          return self(self, root_parse_node->children[0]);
        }

        auto first_terminal =
            *(root_parse_node->children[0]->grammar_symbol.get_terminal_ptr());
        switch (first_terminal) {
        case '(':
          return self(self, root_parse_node->children[1]);
        case '[':
          return self(self, root_parse_node->children[1]);
        case '.':
          return make_complemented_character_class({'\n', '\r'});

        default:
          return std::make_shared<regex::basic_node>(first_terminal);
        }
      } else if (*ptr == "escape-sequence") {
        return std::make_shared<regex::basic_node>(
            parse_escape_sequence(root_parse_node));

      } else if (*ptr == "character-class") {

        std::vector<symbol_type> class_content;

        auto cur_node = root_parse_node;
        while (true) {
          symbol_type cur_symbol = 0;

          if (cur_node->children[0]->grammar_symbol.is_terminal()) {
            cur_symbol =
                *(cur_node->children[0]->grammar_symbol.get_terminal_ptr());
            if (regex_alphabet->is_epsilon(cur_symbol)) {
              break;
            }
          } else {
            cur_symbol = parse_escape_sequence(cur_node->children[0]);
          }
          class_content.push_back(cur_symbol);

          cur_node = cur_node->children[1];
        }

        auto class_size = class_content.size();
        if (class_size == 0) {
          throw cyy::computation::exception::no_regular_expression(
              "empty character class");
        }

        if (class_size == 1) {
          return make_character_class({*(class_content.begin())});
        }

        std::set<symbol_type> symbol_set;

        size_t i = 0;
        bool complemented = false;

        if (class_content[0] == '^') {
          complemented = true;
          i++;
        }

        while (i < class_size) {
          symbol_type cur_symbol = class_content[i];

          if (i + 2 < class_size && class_content[i + 1] == '-') {
            auto end_symbol = class_content[i + 2];

            if (cur_symbol > end_symbol) {
              throw cyy::computation::exception::no_regular_expression(
                  std::string("invalid character range ") +
                  static_cast<char>(cur_symbol) + '-' +
                  static_cast<char>(end_symbol));
            }

            for (symbol_type j = cur_symbol; j <= end_symbol; j++) {
              symbol_set.emplace(j);
            }
            i += 3;
            continue;
          }
          symbol_set.insert(cur_symbol);
          i++;
        }

        if (complemented) {
          return make_complemented_character_class(symbol_set);
        }
        return make_character_class(symbol_set);
      }
    }
    assert(0);
    return {};
  };

  return construct_syntax_tree(construct_syntax_tree, parse_tree);
}
} // namespace cyy::computation
