/*!
 * \file lang.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <stdexcept>

namespace cyy::computation::exception {

  class no_slr_grammar: public std::invalid_argument{
    public:
      using invalid_argument::invalid_argument;


  };
}
