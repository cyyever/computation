/*!
 * \file lang.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <limits>
#include <string_view>

#include "../exception.hpp"
#include "alphabet.hpp"
#include "ascii.hpp"
#include "common_tokens.hpp"
#include "endmarkered_alphabet.hpp"
#include "set_alphabet.hpp"

namespace cyy::computation {

  std::shared_ptr<ALPHABET> ALPHABET::get(std::string_view name,
                                          bool endmarkered) {
    register_factory();
    auto it = factory.find(std::string(name));
    if (it == factory.end()) {
      throw exception::unexisted_alphabet(std::string(name));
    }
    it->second->name = name;
    auto alphabet = it->second;
    if (endmarkered) {
      alphabet = std::make_shared<endmarkered_alphabet>(alphabet);
    }
    return alphabet;
  }

  void ALPHABET::set(const std::shared_ptr<ALPHABET> &alphabet) {
    register_factory();
    factory[alphabet->get_name()] = alphabet;
  }

  void print_symbol_string(std::ostream &os, const symbol_string &str,
                           const ALPHABET &alphabet) {
    for (auto const &c : str) {
      os << alphabet.to_string(c);
    }
  }

  void ALPHABET::register_factory() {
    if (!factory.empty()) {
      return;
    }
    std::shared_ptr<ALPHABET> alphabet;
    alphabet = std::make_shared<common_tokens>();
    factory.emplace(alphabet->get_name(), alphabet);
    alphabet = std::make_shared<printable_ASCII>();
    factory.emplace(alphabet->get_name(), alphabet);
    alphabet = std::make_shared<set_alphabet>(std::set<symbol_type>{'a', 'b'},
                                              "ab_set");
    factory.emplace(alphabet->get_name(), alphabet);
    alphabet = std::make_shared<set_alphabet>(std::set<symbol_type>{'(', ')'},
                                              "parentheses");
    factory.emplace(alphabet->get_name(), alphabet);
    alphabet = std::make_shared<set_alphabet>(std::set<symbol_type>{'0', '1'},
                                              "01_set");
    factory.emplace(alphabet->get_name(), alphabet);
    alphabet =
        std::make_shared<set_alphabet>(std::set<symbol_type>{'0'}, "0_set");
    factory.emplace(alphabet->get_name(), alphabet);
    alphabet = std::make_shared<range_alphabet>(
        0, std::numeric_limits<symbol_type>::max(), "all");
    factory.emplace(alphabet->get_name(), alphabet);
  }
} // namespace cyy::computation
