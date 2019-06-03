/*!
 * \file regex.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <cassert>

#include "../contex_free_lang/cfg_production.hpp"
#include "../contex_free_lang/ll_grammar.hpp"
#include "../exception.hpp"
#include "../lang/set_alphabet.hpp"
#include "regex.hpp"

namespace cyy::computation {

  /*
     rexpr -> rterm rexpr'
     rexpr' -> '|' rterm rexpr'
     rexpr' -> epsilon

     rterm -> rfactor rterm'
     rterm' -> rfactor rterm'
     rterm' -> epsilon

     rfactor -> rprimary rfactor'

     rfactor' -> closure-operator
     rfactor' -> epsilon

     closure-operator -> '*'
     closure-operator -> '+'
     closure-operator -> '?'

     rprimary -> 'non-operator-symbol'
     rprimary -> '.'
     rprimary -> escape-sequence
     rprimary -> '(' rexpr ')'
     rprimary -> '[' character-class ']'

     character-class -> character-class-element character-class'

     character-class' -> character-class-element character-class'
     character-class' -> epsilon

     character-class-element -> 'symbol except backslash and ]'
     character-class-element -> escape-sequence

     escape-sequence -> '\' symbol
     symbol -> 'symbol'
  */
  const LL_grammar &regex::get_grammar(std::string_view alphabet_name) const {
    static std::map<std::string_view, std::shared_ptr<LL_grammar>> factory;
    auto &regex_grammar = factory[alphabet_name];
    if (regex_grammar) {
      return *regex_grammar;
    }

    std::set<CFG::terminal_type> operators{'|', '*', '(', '\\', ')',
                                           '+', '?', '[', ']',  '.'};

    std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
        productions;
    productions["rexpr"] = {{"rterm", "rexpr'"}};
    productions["rexpr'"] = {{'|', "rterm", "rexpr'"}, {}};
    productions["rterm"] = {
        {"rfactor", "rterm'"},
    };
    productions["rterm'"] = {{"rfactor", "rterm'"}, {}};
    productions["rfactor"] = {
        {"rprimary", "rfactor'"},
    };
    productions["rfactor'"] = {{"closure-operator"}, {}};
    productions["closure-operator"] = {
        {'*'},
        {'+'},
        {'?'},
    };
    productions["rprimary"] = {
        {"escape-sequence"},
        {'(', "rexpr", ')'},
        {'[', "character-class", ']'},
        {'.'},
    };

    productions["character-class"] = {
        {"character-class-element", "character-class'"}};

    productions["character-class'"] = {
        {"character-class-element", "character-class'"}, {}};

    productions["character-class-element"] = {{"escape-sequence"}};

    productions["escape-sequence"] = {
        {'\\', "symbol"},
    };

    auto symbol_set = alphabet->get_symbols();
    symbol_set.merge(std::set<symbol_type>(operators));
    auto regex_alphabet = std::make_shared<set_alphabet>(
        symbol_set, alphabet->get_name() + "_regex");
    ALPHABET::set(regex_alphabet);

    regex_alphabet->foreach_symbol([&](auto const &a) {
      productions["symbol"].emplace_back(CFG_production::body_type{a});

      if (!operators.count(a)) {
        productions["rprimary"].emplace_back(CFG_production::body_type{a});
      }

      if (a != '\\' && a != ']') {
        productions["character-class-element"].emplace_back(
            CFG_production::body_type{a});
      }
    });

    regex_grammar = std::make_shared<LL_grammar>(regex_alphabet->get_name(),
                                                 "rexpr", productions);
    return *regex_grammar;
  }

  std::shared_ptr<regex::syntax_node>
  regex::parse(symbol_string_view view) const {

    using syntax_node_ptr = std::shared_ptr<regex::syntax_node>;

    auto escape_symbol = [this](symbol_type symbol) -> symbol_type {
      if (alphabet->contains_ASCII()) {
        switch (symbol) {
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
            return symbol;
        }
      }
      return symbol;
    };

    std::vector<std::shared_ptr<regex::syntax_node>> node_stack;

    symbol_type last_symbol = '\0';

    bool in_class = false;
    bool in_escape_sequence = false;
    std::vector<symbol_type> class_content;
    auto parse_res =
        get_grammar(alphabet->get_name())
            .parse(view, [&node_stack, &last_symbol, &escape_symbol, &in_class,
                          &in_escape_sequence, &class_content,
                          this](auto const &production, auto const &pos) {
              auto const &head = production.get_head();
              auto const &body = production.get_body();
              bool finish_production = (pos == body.size());

              // symbol -> 'symbol'
              if (head == "symbol" && finish_production) {
                last_symbol = *(body[0].get_terminal_ptr());

                if (in_escape_sequence) {
                  last_symbol = escape_symbol(last_symbol);
                }

                if (in_class) {
                  class_content.push_back(last_symbol);
                } else {
                  node_stack.emplace_back(
                      std::make_shared<regex::basic_node>(last_symbol));
                }
              }

              // escape-sequence -> '\' symbol
              if (head == "escape-sequence") {
                in_escape_sequence = (pos == 1);
              }

              // character-class-element -> 'symbol except backslash and ]'
              if (head == "character-class-element") {
                if (finish_production && body[0].is_terminal()) {
                  class_content.push_back(*(body[0].get_terminal_ptr()));
                }
              }

              if (head == "rprimary") {
                // rprimary -> 'non-operator-char'
                // rprimary -> '.'
                if (finish_production && body.size() == 1 &&
                    body[0].is_terminal()) {
                  auto symbol = *(body[0].get_terminal_ptr());
                  if (symbol == '.') {
                    if (alphabet->contains_ASCII()) {
                      node_stack.emplace_back(
                          make_complemented_character_class({'\n', '\r'}));
                    } else {
                      node_stack.emplace_back(
                          make_complemented_character_class({}));
                    }
                  } else {
                    node_stack.emplace_back(
                        std::make_shared<regex::basic_node>(symbol));
                  }
                }
                // rprimary -> '[' character-class ']'
                if (body[0].is_terminal() &&
                    *(body[0].get_terminal_ptr()) == '[') {
                  if (pos == 1) {
                    in_class = true;
                    class_content.clear();
                  } else if (pos == 3) {
                    in_class = false;
                    const auto class_size = class_content.size();
                    assert(class_size != 0);

                    if (class_size == 1) {
                      node_stack.emplace_back(
                          std::make_shared<regex::basic_node>(
                              class_content[0]));
                      return;
                    }

                    std::set<symbol_type> symbol_set;

                    size_t i = 0;
                    bool complemented = false;

                    if (class_content[0] == '^') {
                      complemented = true;
                      i++;
                    }

                    while (i < class_size) {
                      const symbol_type cur_symbol = class_content[i];

                      if (i + 2 < class_size && class_content[i + 1] == '-') {
                        auto end_symbol = class_content[i + 2];

                        if (cur_symbol > end_symbol) {
                          throw cyy::computation::exception::
                              no_regular_expression(
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
                      node_stack.push_back(
                          make_complemented_character_class(symbol_set));
                      return;
                    }
                    node_stack.push_back(make_character_class(symbol_set));
                    return;
                  }
                }
              }
              if (head == "closure-operator" && finish_production) {
                auto const &inner_tree = node_stack.back();
                syntax_node_ptr node;
                auto closure_operator = *(body[0].get_terminal_ptr());
                if (closure_operator == '*') {
                  node =
                      std::make_shared<regex::kleene_closure_node>(inner_tree);
                } else if (closure_operator == '+') {
                  node = std::make_shared<regex::concat_node>(
                      inner_tree,
                      std::make_shared<regex::kleene_closure_node>(inner_tree));
                } else if (closure_operator == '?') {
                  node = std::make_shared<regex::union_node>(
                      std::make_shared<regex::epsilon_node>(), inner_tree);
                } else {
                  assert(0);
                }
                node_stack.back() = node;
              }

              if (head == "rterm'") {
                // rterm' -> rfactor rterm'
                if (body.size() == 2 && pos == 1) {
                  assert(node_stack.size() >= 2);
                  auto right_node = node_stack.back();
                  node_stack.pop_back();
                  auto left_node = node_stack.back();
                  node_stack.pop_back();
                  node_stack.emplace_back(std::make_shared<regex::concat_node>(
                      left_node, right_node));
                }
              }

              if (head == "rexpr'") {
                // rexpr' -> '|' rterm rexpr'
                if (body.size() == 3 && pos == 2) {
                  assert(node_stack.size() >= 2);
                  auto right_node = node_stack.back();
                  node_stack.pop_back();
                  auto left_node = node_stack.back();
                  node_stack.pop_back();
                  node_stack.emplace_back(std::make_shared<regex::union_node>(
                      left_node, right_node));
                }
              }
            });

    if (!parse_res) {
      throw cyy::computation::exception::no_regular_expression("");
    }

    assert(node_stack.size() == 1);
    return node_stack[0];
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

    assert(root);
    return root;
  }

  std::shared_ptr<regex::syntax_node> regex::make_complemented_character_class(
      const std::set<symbol_type> &symbol_set) const {
    std::shared_ptr<regex::syntax_node> root{};

    std::set<symbol_type> complemented_symbol_set;

    alphabet->foreach_symbol([&](auto const &a) {
      if (!symbol_set.count(a)) {
        complemented_symbol_set.insert(a);
      }
    });

    return make_character_class(complemented_symbol_set);
  }

} // namespace cyy::computation
