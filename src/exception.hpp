/*!
 * \file lang.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <stdexcept>

namespace cyy::computation::exception {

class symbol_overflow: public std::invalid_argument {
public:
  using invalid_argument::invalid_argument;
};

class no_regular_expression : public std::invalid_argument {
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
