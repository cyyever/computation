/*!
 * \file ascii.hpp
 *
 * \brief
 * \author cyy
 * \date 2018-03-31
 */
#pragma once

#include "range_alphabet.hpp"

namespace cyy::lang {

class ASCII final : public range_alphabet<0, 127> {
public:
  std::string name() const override { return "ASCII"; }
};

class printable_ASCII final : public range_alphabet<32, 126> {
public:
  std::string name() const override { return "printable-ASCII"; }
};
} // namespace cyy::lang
