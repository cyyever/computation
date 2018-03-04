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

  virtual void foreach_symbol( void(* callback)(const symbol_type & )   )=0;
  virtual bool contain(symbol_type s)=0;
  virtual size_t size()=0;
};

class ASCII final:public alphabet {
  public:
  symbol_type get_epsilon() override {return 128;}

  void foreach_symbol( void(* callback)(const symbol_type & )   ) override {
    for(symbol_type i=0;i<128;i++) {
      callback(i);
    }
  }
  bool contain(symbol_type s) override {return s<128;}
  size_t size() override  {return 128;}

};

} 
