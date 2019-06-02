/*!
 * \file lang.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <string_view>

#include "../exception.hpp"
#include "alphabet.hpp"
#include "ascii.hpp"
#include "common_tokens.hpp"
#include "set_alphabet.hpp"

namespace cyy::computation {

  std::shared_ptr<ALPHABET> ALPHABET::get(std::string_view name) {
    if (factory.empty()) {
      std::shared_ptr<ALPHABET> alphabet;
      alphabet = std::make_shared<common_tokens>();
      factory.emplace(alphabet->get_name(), alphabet);
      alphabet = std::make_shared<printable_ASCII>();
      factory.emplace(alphabet->get_name(), alphabet);
      alphabet = std::make_shared<set_alphabet>(std::set<symbol_type>{'a', 'b'},
                                                "ab_set");
      factory.emplace(alphabet->get_name(), alphabet);
    }

    auto it = factory.find(std::string(name));
    if (it == factory.end()) {
      throw exception::unexisted_alphabet(std::string(name));
    }
    it->second->name = name;
    return it->second;
  }

  void ALPHABET::set(std::shared_ptr<ALPHABET> alphabet) {
    factory[alphabet->get_name()] = alphabet;
  }

  std::set<symbol_type> ALPHABET::get_symbols() const {
    std::set<symbol_type> symbols;
    foreach_symbol([&symbols](auto const s) { symbols.insert(s); });
    return symbols;
  }

  void ALPHABET::print(std::ostream &os, symbol_type symbol) const {
    if (symbol == get_endmarker()) {
      os << "$";
    } else if (contain(symbol)) {
      print_symbol(os, symbol);
    } else {
      os << "(unkown symbol)";
    }
  }

  void print_symbol_string(std::ostream &os, const symbol_string &str,
                           const ALPHABET &alphabet) {
    for (auto const &c : str) {
      alphabet.print(os, c);
    }
  }

} // namespace cyy::computation
