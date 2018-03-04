/*!
 * \file lang.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <map>
#include <set>
#include <string>

namespace cyy::lang {

class alphabet {
public:
  using symbol_type = uint64_t;

  virtual symbol_type get_epsilon()=0;
};

class unicode final:public alphabet {
  public:
  symbol_type get_epsilon() override {return 0;}


};


} // namespace cyy::compiler
