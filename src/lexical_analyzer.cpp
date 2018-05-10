/*!
 * \file lexical_analyzer.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <cassert>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include "lexical_analyzer.hpp"
#include "regex.hpp"

namespace cyy::lang {

void lexical_analyzer::make_NFA() {
  if (nfa_opt) {
    return;
  }

  if (patterns.empty()) {
    throw std::runtime_error("no pattern");
  }

  std::shared_ptr<ALPHABET> alphabet =
      ::cyy::lang::ALPHABET::get(alphabet_name);

  pattern_final_states.clear();

  NFA nfa({0}, alphabet_name, 0, {}, {});

  uint64_t start_state = 1;
  for (auto const &p : patterns) {
    auto sub_nfa = regex(alphabet_name, p.second).to_NFA(start_state);
    REQUIRE(sub_nfa.get_final_states().size() == 1);
    auto final_state = *(sub_nfa.get_final_states().begin());
    nfa.add_sub_NFA(sub_nfa, true);
    pattern_final_states[final_state] = p.first;
    start_state = final_state + 1;
  }
  REQUIRE(nfa.get_final_states().size() == patterns.size());
  nfa_opt = std::move(nfa);
}

std::pair<int, lexical_analyzer::token> lexical_analyzer::scan() {
  make_NFA();
  symbol_string lexeme;
  size_t max_lexeme_size = 0;
  std::set<uint64_t> prev_final_set;

  auto cur_set = nfa_opt->get_start_epsilon_closure();

  auto line_no = cur_line;
  auto column_no = cur_column;

  while (true) {
    symbol_type c = 0;
    input_stream.get(c);

    if (!input_stream) {
      break;
    }

    cur_set = nfa_opt->move(cur_set, c);
    lexeme.push_back(c);

    if (c == '\n') {
      cur_line++;
      cur_column = 1;
    } else {
      cur_column++;
    }

    if (nfa_opt->contain_final_state(cur_set)) {
      prev_final_set = cur_set;
      max_lexeme_size = lexeme.size();
    }
  }

  if (max_lexeme_size) {
    lexeme.resize(max_lexeme_size);

    token t;
    for (auto const &[final_state, token_name] : pattern_final_states) {
      if (prev_final_set.count(final_state)) {
        t.name = token_name;
        t.attribute.line_no = line_no;
        t.attribute.column_no = column_no;
        t.attribute.lexeme = std::move(lexeme);
        return {0, t};
      }
    }
    assert(0);
  }

  if (!input_stream && !input_stream.eof()) {
    return {-2, {}};
  }

  if (!lexeme.empty()) {
    return {1, {}};
  }
  return {-1, {}};
}

} // namespace cyy::lang
