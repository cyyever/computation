/*!
 * \file lang.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include "lang.hpp"

namespace cyy::lang {

class ASCII final : public ALPHABET {
public:
  symbol_type get_epsilon() override { return 128; }

  // void foreach_symbol(void (*callback)(const symbol_type &)) override {
  void foreach_symbol(
      const std::function<void(const symbol_type &)> &callback) const override {
    for (symbol_type i = 0; i < 128; i++) {
      callback(i);
    }
  }
  bool contain(symbol_type s) const override { return s < 128; }
  size_t size() const override { return 128; }
  std::string name() const override { return "ASCII"; }
};

std::unique_ptr<ALPHABET> make_alphabet(const std::string &name) {
  if (name == "ASCII") {
    return std::unique_ptr<ALPHABET>{new ASCII()};
  }
  throw std::invalid_argument(std::string("unkown alphabet name:") + name);
}
} // namespace cyy::lang
