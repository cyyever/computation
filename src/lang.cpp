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
  symbol_type get_epsilon() const override { return 128; }
  symbol_type get_endmarker() const override {return 129;}

    void print(std::ostream& os,symbol_type symbol) const override {
    if(symbol==get_epsilon()) {
	os<<"'epsilon'";
    } else if(symbol==get_endmarker()) {
	os<<"$";
    } else {
	os<<'\''<<static_cast<char>(symbol)<<'\'';
    }
    return;
  }

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
