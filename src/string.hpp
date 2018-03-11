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
 template< class CharT>
   CharT * KMP(std::basic_string_view<CharT> word,std::basic_string_view<CharT> str) {
    if(word.empty()|| word.size()>str.size()) {
      return nullptr;
    }

    //for each substring w1...ws,compute the longest proper prefix w1...wf(s) that is a suffix of w1...ws
    std::vector<size_t>  failure_function(word.size(),0);
    //f[1] is always empty string,so we begin with w2;
    for(size_t i=1;i<word.size();i++) {
      auto t=failure_function[i-1];
      while(t>0 && word[t] !=word[i]) 
      {
	t=failure_function[t];
      }
      if(word[t]==word[i]) {
	failure_function[i]=t+1;
      }
    }

    size_t s=0;
    for(size_t i=0;i<str.size();i++) {
      auto c=str[i];
      while(s>0 && word[s]!=c) {
	s=failure_function[s];
      }
      if(word[s]==c) {
	s++;
	if(s==word.size()) {
	  return str.data()+i-word.size()+1;
	}
      }
    }

    return nullptr;
  }
} // namespace cyy::lang
