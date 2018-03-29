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
  symbol_type get_endmarker() const override { return 129; }

  void print(std::ostream &os, symbol_type symbol) const override {
    if (symbol == get_epsilon()) {
      os << "'epsilon'";
    } else if (symbol == get_endmarker()) {
      os << "$";
    } else {
      os << '\'' << static_cast<char>(symbol) << '\'';
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

class example_tokens final : public ALPHABET {
public:
  symbol_type get_epsilon() const override { return example_token::epsilon; }
  symbol_type get_endmarker() const override { return example_token::endmark; }

  void print(std::ostream &os, symbol_type symbol) const override {
    if (symbol == get_epsilon()) {
      os << "'epsilon'";
    } else if (symbol == get_endmarker()) {
      os << "$";
    } else {
      os << '\'' << static_cast<char>(symbol) << '\'';
    }
    return;
  }

  void foreach_symbol(
      const std::function<void(const symbol_type &)> &callback) const override {
    for (symbol_type i = 0; i <256; i++) {
      callback(i);
    }
    for (symbol_type i = example_token::epsilon+1; i <example_token::endmark; i++) {
      callback(i);
    }
  }
  bool contain(symbol_type s) const override { return s <256 || s>example_token::epsilon && s<example_token::endmark; }
  size_t size() const override { return 256+example_token::endmark-example_token::epsilon-1; }
  std::string name() const override { return "example_tokens"; }
};


std::unique_ptr<ALPHABET> make_alphabet(const std::string &name) {
  if (name == "ASCII") {
    return std::unique_ptr<ALPHABET>{new ASCII()};
  }
  if (name == "example_tokens") {
    return std::unique_ptr<ALPHABET>{new example_tokens()};
  }
  throw std::invalid_argument(std::string("unkown alphabet name:") + name);
}
} // namespace cyy::lang
