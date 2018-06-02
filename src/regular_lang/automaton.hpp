/*!
 * \file automaton.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <map>
#include <optional>
#include <set>
#include <string>

#include "../lang/lang.hpp"

namespace cyy::computation {

class finite_automaton {
public:
  finite_automaton(const std::set<uint64_t> &states_,
                   const std::string &alphabet_name, uint64_t start_state_,
                   const std::set<uint64_t> &final_states_)
      : alphabet(::cyy::computation::ALPHABET::get(alphabet_name)),
        states(states_), start_state(start_state_),
        final_states(final_states_) {

    if (states.empty()) {
      throw std::invalid_argument("no state");
    }
    if (!states.count(start_state)) {
      throw std::invalid_argument("unexisted start state");
    }
    for (auto const &final_state : final_states) {
      if (!states.count(final_state)) {
        throw std::invalid_argument(std::string("unexisted start state ") +
                                    std::to_string(final_state));
      }
    }
  }

  auto get_states() const -> auto const & { return states; }
  auto get_alphabet() const -> auto const & { return *alphabet; }
  auto get_final_states() const -> auto const & { return final_states; }
  uint64_t get_start_state() const { return start_state; }

  bool operator==(const finite_automaton &rhs) const {
    return (this == &rhs) ||
           (alphabet == rhs.alphabet && states == rhs.states &&
            start_state == rhs.start_state && final_states == rhs.final_states);
  }

  bool contain_final_state(const std::set<uint64_t> &T) const {
    for (const auto &f : final_states) {
      if (T.count(f) == 1) {
        return true;
      }
    }
    return false;
  }

protected:
  std::shared_ptr<ALPHABET> alphabet;
  std::set<uint64_t> states;
  uint64_t start_state;
  std::set<uint64_t> final_states;
};

} // namespace cyy::computation
