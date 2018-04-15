/*!
 * \file lexical_analyzer.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#pragma once

#include <vector>

#include "automaton.hpp"
#include "lang.hpp"

namespace cyy::lang {

class lexical_analyzer {

  lexical_analyzer(const std::string &alphabet_name, 
		  const std::vector<symbol_string> & regex_exprs )
      : alphabet(ALPHABET::get(alphabet_name)) {
  }
};
} // namespace cyy::lang
