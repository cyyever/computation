/*!
 * \file ascii.hpp
 *
 * \brief
 * \author cyy
 * \date 2018-03-31
 */
#pragma once

#include "range_alphabet.hpp"

namespace cyy::computation {

  class ASCII final : public range_alphabet<0, 127> {};

  class printable_ASCII final : public range_alphabet<32, 126> {};
} // namespace cyy::computation
