/*!
 * \file union_alphabet.hpp
 *
 */
#pragma once

#include <iostream>
#include <memory>

#include "alphabet.hpp"

namespace cyy::computation {

  class endmarked_alphabet : public ALPHABET {
  public:
    endmarked_alphabet(std::shared_ptr<ALPHABET> alphabet_)
        : ALPHABET("placeholder"), alphabet{alphabet_} {
      if (alphabet->contain(ALPHABET::endmarker)) {
        has_endmarker = true;
        set_name(alphabet->get_name());
      } else {
        set_name(std::string("endmarked_") + alphabet->get_name());
      }
    }

    bool contain(symbol_type s) const noexcept override {
      return alphabet->contain(s) || s == ALPHABET::endmarker;
    }

    size_t size() const noexcept override {
      size_t real_size = 0;
      if (!has_endmarker) {
        real_size++;
      }

      real_size += alphabet->size();
      return real_size;
    }
    auto original_alphabet() const { return alphabet; }

  private:
    symbol_type get_symbol(size_t index) const noexcept override {
      if (index + 1 == size()) {
        return ALPHABET::endmarker;
      }
      auto it = alphabet->begin();
      return *(it + index);
    }

  private:
    std::shared_ptr<ALPHABET> alphabet;
    bool has_endmarker{false};
  };
} // namespace cyy::computation
