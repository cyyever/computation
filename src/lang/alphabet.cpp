/*!
 * \file lang.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <map>
#include <string_view>

#include "../exception.hpp"
#include "alphabet.hpp"
#include "ascii.hpp"
#include "common_tokens.hpp"
#include "set_alphabet.hpp"

namespace cyy::computation {

  std::shared_ptr<ALPHABET> ALPHABET::get(std::string_view name) {
    static const std::map<std::string_view, std::shared_ptr<ALPHABET>> factory =
        {
            {"common_tokens", std::make_shared<common_tokens>()},
            {"ASCII", std::make_shared<ASCII>()},
            {"printable-ASCII", std::make_shared<printable_ASCII>()},
            {"ab_set",
             std::make_shared<set_alphabet>(std::set<symbol_type>{'a', 'b'})},
        };
    auto it = factory.find(name);
    if (it == factory.end()) {
      throw exception::unexisted_alphabet(std::string(name));
    }
    it->second->name = name;
    return it->second;
  }

  void ALPHABET::print(std::ostream &os, symbol_type symbol) const {
    if (symbol == get_endmarker()) {
      os << "$";
    } else if (contain(symbol)) {
      print_symbol(os, symbol);
    } else {
      os << "(unkown symbol)";
    }
    return;
  }

  void print_symbol_string(std::ostream &os, const symbol_string &str,
                           const ALPHABET &alphabet) {
    for (auto const &c : str) {
      alphabet.print(os, c);
    }
  }

} // namespace cyy::computation
