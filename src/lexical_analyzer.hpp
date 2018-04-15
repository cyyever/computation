/*!
 * \file lexical_analyzer.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "automaton.hpp"
#include "lang.hpp"

namespace cyy::lang {

class lexical_analyzer {
public:
  struct token_attribute {
    size_t line_no;
    size_t column_no;
    symbol_string lexeme;
  };

  struct token {
    symbol_type name;
    token_attribute attribute;
  };

public:
  explicit lexical_analyzer(const std::string &alphabet_name_)
      : alphabet_name(alphabet_name_) {}

  void append_pattern(const symbol_type &token_name, symbol_string pattern) {
    patterns.emplace_back(token_name, std::move(pattern));
    nfa_opt.reset();
  }

  void set_input_stream(symbol_istringstream &&is) {
    cur_line = 1;
    cur_column = 1;
    input_stream = std::move(is);
  }

  //! \brief scan the input stream,return first token
  //! \return
  //	when successed,return 0 and the token
  //	when no token in remain input,return 1 and null token
  //	when EOF was seen,return -1 and null token
  //	when failed,return -2 and null token
  //! \note when no successed, stream is not consumed
  std::pair<int, token> scan();

private:
  void make_NFA();

private:
  std::string alphabet_name;
  std::vector<std::pair<symbol_type, symbol_string>> patterns;
  size_t cur_line{1};
  size_t cur_column{1};

  symbol_istringstream input_stream;
  std::optional<NFA> nfa_opt;
  std::map<uint64_t, symbol_type> pattern_final_states;
};
} // namespace cyy::lang
