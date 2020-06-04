/*!
 * \file lang.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */
#pragma once

#include "range_alphabet.hpp"

namespace cyy::computation {

  enum class common_token : symbol_type {
    ascii_char = 256,
    escape_sequence,
    digit,
    number,
    whitespace,
    id,
    INT,
    FLOAT,
    record,
    CLASS,
    __end,
  };

  class common_tokens final : public range_alphabet {
  public:
    common_tokens()
        : range_alphabet(0, static_cast<symbol_type>(common_token::__end) - 1,
                         "common_tokens") {}

  private:
    std::string to_string(symbol_type symbol) const override;
    bool contains_ASCII() const override;
  };

} // namespace cyy::computation
