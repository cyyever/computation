/*!
 * \file map_alphabet.hpp
 *
 * \brief
 * \author cyy
 * \date 2018-03-31
 */
#pragma once

#include "alphabet.hpp"

namespace cyy::computation {
  class map_alphabet final : public ALPHABET {
  public:
    map_alphabet(std::map<symbol_type, std::string> symbol_map_,
                 std::string_view name_)
        : ALPHABET(name_), symbol_map(std::move(symbol_map_)) {
      if (symbol_map.empty()) {
        throw exception::empty_alphabet("symbol map is empty");
      }
    }
    bool contain(symbol_type s) const noexcept override {
      return symbol_map.contains(s);
    }
    size_t size() const noexcept override { return symbol_map.size(); }

    std::string MMA_draw() const {
      std::string cmd = "{";
      for (auto const &[s, _] : symbol_map) {
        cmd += ALPHABET::MMA_draw(s);
        cmd.push_back(',');
      }
      cmd.back() = '}';
      return cmd;
    }

    std::string get_data(symbol_type symbol) const {
      return symbol_map.at(symbol);
    }

  private:
    std::string __to_string(symbol_type symbol) const override {
      return get_data(symbol);
    }
    symbol_type get_symbol(size_t index) const noexcept override {
      auto it = symbol_map.begin();
      std::advance(it,index);
      return it->first;
    }

  private:
    std::map<symbol_type, std::string> symbol_map;
  };

} // namespace cyy::computation
