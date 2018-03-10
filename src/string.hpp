/*!
 * \file lang.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include "lang.hpp"

namespace cyy::lang {

  // find word from str
  inline symbol_string_view  KMP(symbol_string_view word,symbol_string_view str) {
    if(word.empty()|| word.size()>str.size()) {
      return {};
    }

    //for each substring w1...ws,compute the longest proper prefix w1...wf(s) that is a suffix of w1...ws
    std::vector<size_t>  failure_function(word.size(),0);
    //f[1] is always empty string,so we begin with w2;
    for(size_t i=1;i<word.size();i++) {
      auto t=failure_function[i-1];
      while(t!=0 && word[t] !=word[i]) 
      {
	t=failure_function[t];
      }
      if(word[t]==word[i]) {
	failure_function[i]=t+1;
      }
    }

    size_t i=0;
    while(!str.empty()) {
      if(word[i]==str[i]) {
	i++;
	if(i==word.size()) {
	  return {str.data(),word.size()};
	}
	continue;
      } 
      auto fail_pos=failure_function[i];
      str.remove_prefix(i+1-fail_pos);
      i=fail_pos-1;
    }
    return {};
  }
} // namespace cyy::lang
