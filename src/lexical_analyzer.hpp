/*!
 * \file lexical_analyzer.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <vector>
#include <string>
#include <optional>

#include "automaton.hpp"
#include "lang.hpp"

namespace cyy::lang {

class lexical_analyzer {
  public:
  struct token_attribute {
    size_t line_no;
    size_t colunm_no;
    std::string lexeme;
  };

  struct token {
    symbol_type name;
    token_attribute attribute;
  };

public:
 explicit lexical_analyzer(const std::string &alphabet_name_)
    : alphabet_name(alphabet_name_) { }

  void append_pattern(const
     symbol_type  &token_name,
     symbol_string pattern) {
    patterns.emplace_back(token_name,std::move(pattern));
    nfa_opt.reset();
  }

  //! \brief scan the input stream,return first token
  //! \return 
  //	when successed,return 0 and the token
  //	when EOF was seen,return -1 and null token
  //	when failed,return -2 and null token
  //! \note when no successed, stream is not consumed
  std::pair<int,token> scan(symbol_istringstream &is); 
 
private:
  void make_NFA();

private:
  std::string alphabet_name;
  std::vector<std::pair<symbol_type,symbol_string>> patterns;
  size_t cur_line{1};
  size_t cur_colunm{1};

  std::optional<NFA> nfa_opt;
  std::map<uint64_t,symbol_type> pattern_final_states;
};
} // namespace cyy::lang
