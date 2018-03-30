/*!
 * \file lang.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <map>
#include "lang.hpp"
#include "common_tokens.hpp"

namespace cyy::lang {

  std::shared_ptr<ALPHABET> make_alphabet(const std::string &name) {
  static std::map<std::string,std::shared_ptr<ALPHABET>> factory= {
    {"common_tokens",std::make_shared<common_tokens>()}
  };

  auto ptr=factory[name];
  if(!ptr) {
    throw std::invalid_argument(std::string("unkown alphabet name:") + name);
  }
  return ptr;
}
} // namespace cyy::lang
