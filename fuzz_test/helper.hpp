/*!
 * \file helper.hpp
 *
 * \brief fuzzing helper functions
 * \author cyy
 * \date 2019-02-12
 */
#pragma once

#include "context_free_lang/cfg.hpp"
#include "regular_lang/nfa.hpp"

inline char to_printable_ASCII(uint8_t data) { return data % 95 + 32; }

cyy::computation::CFG::production_set_type
fuzzing_CFG_productions(const uint8_t *Data, size_t Size);

cyy::computation::NFA fuzzing_NFA(const uint8_t *Data, size_t Size);
cyy::computation::symbol_string fuzzing_symbol_string(const uint8_t *Data,
                                                      size_t Size);
