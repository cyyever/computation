/*!
 * \file regex.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <cyy/algorithm/alphabet/range_alphabet.hpp>

#include "exception.hpp"
#include "regex.hpp"

namespace cyy::computation {

  namespace {
    class character_class {
    public:
      character_class() = default;
      void reset() {
        content.clear();
        in_range = false;
        last_symbol = {};
        range_begin = {};
      }

      void add_symbol(symbol_type s) {
        if (in_range) {
          if (content.empty() || s < range_begin) {
            throw cyy::computation::exception::no_regular_expression(
                std::string("invalid character range ") +
                static_cast<char>(range_begin) + '-' + static_cast<char>(s));
          }
          for (auto i = range_begin; i <= s; i++) {
            content.insert(i);
          }
          in_range = false;
        }
        last_symbol = s;
        content.insert(s);
      }
      bool set_last_symbol_in_range() {
        if (in_range || content.empty()) {
          return false;
        }

        in_range = true;
        range_begin = last_symbol;
        return true;
      }
      /* auto get_content() const -> const auto & { return content; } */
      const auto &get_content() const { return content; }

    private:
      symbol_set_type content;
      symbol_type last_symbol{};
      symbol_type range_begin{};
      bool in_range{false};
    };
  } // namespace

  /*
     rexpr -> epsilon
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
     character-class -> '^' character-class'

     character-class' -> character-class-element character-class'
     character-class' -> '-' character-class-element character-class'
     character-class' -> epsilon

     character-class-element -> 'symbol except \ and ] and - and ^'
     character-class-element -> escape-sequence

     escape-sequence -> '\' symbol
     symbol -> 'symbol'
  */

  const LL_grammar &regex::get_grammar() const {
    static std::unordered_map<std::string, std::shared_ptr<LL_grammar>> factory;
    auto &regex_grammar = factory[alphabet->get_name()];
    if (regex_grammar) {
      return *regex_grammar;
    }

    symbol_set_type const operators{'|', '*', '(', '\\', ')', '+',
                                    '?', '[', ']', '.',  '^', '-'};

    CFG::production_set_type productions;
    productions["rexpr"] = {{"rterm", "rexpr'"}, {}};
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

    productions["character-class"].emplace(
        CFG_production::body_type{'^', "character-class'"});
    productions["character-class'"] = {
        {'-', "character-class-element", "character-class'"},
        {"character-class-element", "character-class'"},
        {}};

    productions["character-class-element"] = {{"escape-sequence"}};

    productions["escape-sequence"] = {
        {'\\', "symbol"},
    };

    for (auto a : alphabet->get_view()) {
      productions["symbol"].emplace(CFG_production::body_type{a});

      if (!operators.contains(a)) {
        productions["rprimary"].emplace(CFG_production::body_type{a});
      }

      if (a != '\\' && a != ']' && a != '-' && a != '^') {
        productions["character-class-element"].emplace(
            CFG_production::body_type{a});
      }
    }

    symbol_set_type symbol_set;
    for (auto const s : alphabet->get_view()) {
      symbol_set.insert(s);
    }

    symbol_set.merge(symbol_set_type(operators));
    auto regex_alphabet = std::make_shared<cyy::algorithm::set_alphabet>(
        symbol_set, alphabet->get_name() + "_regex");
    ALPHABET::set(regex_alphabet);

    regex_grammar =
        std::make_shared<LL_grammar>(regex_alphabet, "rexpr", productions);
    return *regex_grammar;
  }

  symbol_type regex::escape_symbol(symbol_type symbol) const {
    if (alphabet->support_ASCII_escape_sequence()) {
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
  }

  std::shared_ptr<regex::syntax_node>
  regex::parse(symbol_string_view view) const {

    using syntax_node_ptr = std::shared_ptr<regex::syntax_node>;

    std::vector<std::shared_ptr<regex::syntax_node>> node_stack;

    bool in_class = false;
    bool in_complemented_class = false;
    bool in_escape_sequence = false;
    character_class cls;
    auto const parse_res = get_grammar().parse(
        view,
        [&node_stack, &in_class, &cls, &in_complemented_class,
         &in_escape_sequence, this](auto const &production, auto const &pos) {
          auto const &head = production.get_head();
          auto const &body = production.get_body();
          const bool finish_production = (pos == body.size());

          // symbol -> 'symbol'
          if (head == "symbol" && finish_production) {
            auto s = body[0].get_terminal();
            if (in_escape_sequence) {
              s = escape_symbol(s);
            }

            if (in_class) {
              cls.add_symbol(s);
            } else {
              node_stack.emplace_back(std::make_shared<regex::basic_node>(s));
            }
            return;
          }

          // escape-sequence -> '\' symbol
          if (head == "escape-sequence") {
            in_escape_sequence = (pos == 1);
          }

          // character-class-element -> 'symbol except backslash and ]'
          if (head == "character-class-element") {
            if (finish_production && body[0].is_terminal()) {
              cls.add_symbol(body[0].get_terminal());
            }
          }

          // character-class -> '^' character-class'
          if (head == "character-class" && pos == 0 && body[0] == '^') {
            in_complemented_class = true;
          }

          // character-class' -> '-' character-class-element
          // character-class'
          if (head == "character-class'") {
            if (pos == 1 && body[0] == '-') {
              if (!cls.set_last_symbol_in_range()) {
                throw cyy::computation::exception::no_regular_expression(
                    std::string("invalid character range "));
              }
            }
          }

          if (head == "rprimary") {
            // rprimary -> 'non-operator-char'
            // rprimary -> '.'
            if (finish_production && body.size() == 1 &&
                body[0].is_terminal()) {
              auto symbol = body[0].get_terminal();
              if (symbol == '.') {
                if (alphabet->support_ASCII_escape_sequence()) {
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
              return;
            }

            // rprimary -> '[' character-class ']'
            if (body[0] == '[') {
              if (pos == 1) {
                cls.reset();
                in_class = true;
                in_complemented_class = false;
              } else if (pos == 3) {
                in_class = false;
                const auto &class_content = cls.get_content();
                if (class_content.empty()) {
                  throw cyy::computation::exception::no_regular_expression(
                      "empty character class");
                }

                if (in_complemented_class) {
                  node_stack.push_back(
                      make_complemented_character_class(class_content));
                  return;
                }
                node_stack.push_back(make_character_class(class_content));
                return;
              }
              return;
            }
          }
          if (head == "closure-operator" && finish_production) {
            auto const &inner_tree = node_stack.back();
            syntax_node_ptr node;
            const auto closure_operator = body[0].get_terminal();
            if (closure_operator == '*') {
              node = std::make_shared<regex::kleene_closure_node>(inner_tree);
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
              node_stack.emplace_back(
                  std::make_shared<regex::concat_node>(left_node, right_node));
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
              node_stack.emplace_back(
                  std::make_shared<regex::union_node>(left_node, right_node));
            }
          }

          if (head == "rexpr") {
            // rexpr-> epsilon
            if (pos == 0 && body.empty()) {
              node_stack.emplace_back(std::make_shared<regex::epsilon_node>());
              return;
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
  regex::make_character_class(const symbol_set_type &symbol_set) {
    std::shared_ptr<regex::syntax_node> root{};
    for (auto const symbol : symbol_set) {
      if (!root) {
        root = std::make_shared<regex::basic_node>(symbol);
      } else {
        root = std::make_shared<regex::union_node>(
            std::move(root), std::make_shared<regex::basic_node>(symbol));
      }
    }

    assert(root);
    return root;
  }

  std::shared_ptr<regex::syntax_node> regex::make_complemented_character_class(
      const symbol_set_type &symbol_set) const {
    std::shared_ptr<regex::syntax_node> const root{};

    symbol_set_type complemented_symbol_set;

    for (const auto a : alphabet->get_view()) {
      if (!symbol_set.contains(a)) {
        complemented_symbol_set.insert(a);
      }
    }

    return make_character_class(complemented_symbol_set);
  }

} // namespace cyy::computation
