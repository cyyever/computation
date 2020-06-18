/*!
 * \file union_alphabet.hpp
 *
 */
#pragma once

#include <iostream>
#include <memory>

#include "alphabet.hpp"

namespace cyy::computation {

  class union_alphabet : public ALPHABET {
  public:
    union_alphabet(std::shared_ptr<ALPHABET> alphabet1_, std::shared_ptr<ALPHABET> alphabet2_,
                   std::string_view name_ = "")
        : ALPHABET("placeholder"),alphabet1{alphabet1_},alphabet2{alphabet2_} {
      if (name_.empty()) {
        set_name(alphabet1->get_name() + "_union_" + alphabet2->get_name());
      }
    }

    bool contain(symbol_type s) const noexcept override {
      return alphabet1->contain(s) || alphabet2->contain(s);
    }

    size_t size() const noexcept override {
      return alphabet1->size() + alphabet2->size();
    }

    symbol_type get_symbol(size_t index) const noexcept override {
      auto alphabet1_size = alphabet1->size();
      if (index < alphabet1_size) {
        auto it1=alphabet1->begin();
      return *(it1+index);
      }
      auto it2=alphabet2->begin();
      return *(it2+(index-alphabet1_size));
    }

  private:
    std::shared_ptr<ALPHABET>alphabet1;
    std::shared_ptr<ALPHABET>alphabet2;
  };
} // namespace cyy::computation
