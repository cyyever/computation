/*!
 * \file lang.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include "lang.hpp"
#include "common_tokens.hpp"
#include "ascii.hpp"
#include "set_alphabet.hpp"

namespace cyy::lang {

std::map<std::string,std::shared_ptr<ALPHABET>> ALPHABET::factory= {
    {"common_tokens",std::make_shared<common_tokens>()},
    {"ASCII",std::make_shared<ASCII>()},
    {"ab_set",std::make_shared<set_alphabet>( std::set<symbol_type>{'a','b'}     )},
  };

std::shared_ptr<ALPHABET> ALPHABET::get(const std::string &name) {
  auto ptr=ALPHABET::factory[name];
  if(!ptr) {
    throw std::invalid_argument(std::string("unkown alphabet name:") + name);
  }
  ptr->alternative_name=name;
  return ptr;
}
} // namespace cyy::lang
