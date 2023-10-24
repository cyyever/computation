
#include "automaton.hpp"

#include <ranges>
#include <sstream>

namespace cyy::computation {
  bool finite_automaton::state_set_type::has_intersection(
      const state_set_type &rhs) const {
    auto it = begin();
    auto it2 = rhs.begin();
    while (it != end() && it2 != rhs.end()) {
      if (*it == *it2) {
        return true;
      }
      if (*it < *it2) {
        ++it;
      } else {
        ++it2;
      }
    }
    return false;
  }

  finite_automaton::state_bitset_type
  finite_automaton::get_bitset(uint64_t bitset_value) const {
    return state_bitset_type(states.size(), bitset_value);
  }

  finite_automaton::state_bitset_type
  finite_automaton::get_bitset(const state_set_type &state_set) const {
    state_bitset_type bitset(states.size());
    auto it = states.begin();
    auto it2 = state_set.begin();
    while (it != states.end() && it2 != state_set.end()) {
      if (*it == *it2) {
        bitset.set(static_cast<size_t>(std::distance(states.begin(), it)));
        ++it;
        ++it2;
        continue;
      }
      if (*it < *it2) {
        ++it;
        continue;
      }
      ++it2;
    }
    return bitset;
  }

  finite_automaton::state_set_type
  finite_automaton::from_bitset(const state_bitset_type &bitset) const {
    state_set_type result;
    size_t n = 0;
    for (auto const state : states) {
      if (bitset.test(n)) {
        result.emplace(state);
      }
      n++;
    }
    return result;
  }

  bool
  finite_automaton::state_bitset_contains(const state_bitset_type &state_bitset,
                                          state_type state) const {
    return state_bitset.test(
        static_cast<size_t>(std::distance(states.begin(), states.find(state))));
  }

  std::string finite_automaton::MMA_draw() const {
    std::stringstream is;
    is << "GraphLayout -> \"CircularEmbedding\", ImageSize -> "
          "Large,VertexLabels -> "
          "Placed[\"Name\",Center],VertexSize -> "
          "0.1,VertexShapeFunction -> {"
       << get_start_state() << " -> \"Square\"},VertexStyle -> {Gray";
    for (auto const s : get_final_states()) {
      is << "," << s << "-> Green";
    }
    is << '}';
    return is.str();
  }

  finite_automaton::symbol_set_type
  finite_automaton::get_state_symbol_set() const {
    symbol_set_type state_symbol_set;
    for (auto const s : states) {
      assert(s <= std::numeric_limits<cyy::algorithm::symbol_type>::max());
      state_symbol_set.insert(static_cast<cyy::algorithm::symbol_type>(s));
    }
    return state_symbol_set;
  }

  finite_automaton::state_set_product_type
  finite_automaton::get_state_set_product(
      const state_set_type &another_state_set) const {
    state_set_product_type product;
    state_type next_state = 0;
    for (auto s1 : get_states()) {
      for (auto s2 : another_state_set) {
        product.try_emplace({s1, s2}, next_state);
        next_state++;
      }
    }
    return product;
  }
} // namespace cyy::computation
