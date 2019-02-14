/*!
 * \file lang.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include "lang.hpp"
#include "ascii.hpp"
#include "common_tokens.hpp"
#include "set_alphabet.hpp"

namespace cyy::computation {

  std::shared_ptr<ALPHABET> ALPHABET::get(const std::string &name) {
    static const std::map<std::string, std::shared_ptr<ALPHABET>> factory = {
        {"common_tokens", std::make_shared<common_tokens>()},
        {"ASCII", std::make_shared<ASCII>()},
        {"printable-ASCII", std::make_shared<printable_ASCII>()},
        {"ab_set",
         std::make_shared<set_alphabet>(std::set<symbol_type>{'a', 'b'})},
    };
    auto it = factory.find(name);
    if (it == factory.end()) {
      throw std::invalid_argument(std::string("unkown alphabet name:") + name);
    }
    it->second->name = name;
    return it->second;
  }
} // namespace cyy::computation
