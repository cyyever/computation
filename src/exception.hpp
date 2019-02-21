/*!
 * \file lang.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <stdexcept>

namespace cyy::computation::exception {

  class symbol_overflow : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class unexisted_alphabet : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class no_finite_automaton : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class unexisted_finite_automaton_state : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class no_regular_expression : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class no_CFG : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class invalid_CFG_production : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class no_LL_grammar : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };
  class no_SLR_grammar : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };
  class no_canonical_LR_grammar : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class no_LALR_grammar : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };
} // namespace cyy::computation::exception
