/*!
 * \file helper.cpp
 *
 * \brief fuzzing helper functions
 * \author cyy
 * \date 2019-02-12
 */
#include "helper.hpp"

using namespace cyy::computation;
std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>> fuzzing_CFG_productions(const uint8_t *Data, size_t Size) {
  std::map<CFG::nonterminal_type, std::vector<CFG_production::body_type>>
    productions;

  size_t i=0;
  while(i<Size) {
  CFG::nonterminal_type header;
    if(i<Size) {
      header=CFG::nonterminal_type(1,to_printable_ASCII(Data[i]));
      i++;
    }

    if(i<Size) {
      CFG_production::body_type body;
      size_t body_size=Data[i];
      i++;
      for(size_t j=0;j<body_size && i<Size;j++,i+=2) {
        if(i+1<Size) {
          if(Data[i]%2==0) {
            body.emplace_back( grammar_symbol_type::nonterminal_type(1, to_printable_ASCII(Data[i+1])));
          } else {
            body.push_back( grammar_symbol_type::terminal_type( to_printable_ASCII(Data[i+1])));
          }
        }
      }
      productions[header].emplace_back(std::move(body));
    }
  }
  return productions;
}
