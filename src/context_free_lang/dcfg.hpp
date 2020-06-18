/*!
 * \file dcfg.hpp
 *
 */

#pragma once

#include "cfg.hpp"

namespace cyy::computation {

  class DCFG : public CFG {

  public:
    DCFG(const std::string &alphabet_name, nonterminal_type start_symbol_,
         production_set_type productions_)
        : CFG(alphabet_name, start_symbol_, std::move(productions_)) {
      if (!DK_test()) {
        throw exception::no_DCFG("DK test failed");
      }
    }

  private:
    bool DK_test() const;
  };
} // namespace cyy::computation
