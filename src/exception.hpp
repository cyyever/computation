/*!
 * \file exception.hpp
 *
 * \brief
 */

#pragma once

#include <stdexcept>

namespace cyy::computation::exception {

  class symbol_overflow : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class empty_alphabet_name : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };
  class empty_alphabet : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };
  class invalid_alphabet : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class empty_syntax_tree : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class unexisted_alphabet : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class unmatched_alphabets : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class no_finite_automaton : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };

  class no_DFA : public std::invalid_argument {
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
  class left_recursion_CFG : public std::invalid_argument {
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
  class no_CNF : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };
  class pop_empty_stack : public std::logic_error {
  public:
    using logic_error::logic_error;
  };
  class no_DPDA : public std::invalid_argument {
  public:
    using invalid_argument::invalid_argument;
  };
} // namespace cyy::computation::exception
