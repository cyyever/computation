/*!
 * \file automaton.hpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#pragma once

#include <map>
#include <optional>
#include <range/v3/algorithm.hpp>
#include <set>
#include <string>
#include <string_view>

#include "../lang/alphabet.hpp"

namespace cyy::computation {

  class finite_automaton {
  public:
    using state_type = uint64_t;
    finite_automaton(const std::set<state_type> &states_,
                     std::string_view alphabet_name, state_type start_state_,
                     const std::set<state_type> &final_states_)
        : alphabet(::cyy::computation::ALPHABET::get(alphabet_name)),
          states(states_), start_state(start_state_),
          final_states(final_states_) {

      if (states.empty()) {
        throw cyy::computation::exception::no_finite_automaton("no state");
      }
      if (!states.count(start_state)) {
        throw cyy::computation::exception::no_finite_automaton(
            "unexisted start state");
      }
      for (auto const &final_state : final_states) {
        if (!states.count(final_state)) {
          throw cyy::computation::exception::no_finite_automaton(
              std::string("unexisted state ") + std::to_string(final_state));
        }
      }
    }

    auto get_states() const noexcept -> auto const & { return states; }
    auto get_alphabet() const noexcept -> auto const & { return *alphabet; }
    auto get_final_states() const noexcept -> auto const & {
      return final_states;
    }
    state_type get_start_state() const noexcept { return start_state; }

    bool operator==(const finite_automaton &rhs) const {
      return (this == &rhs) ||
             (alphabet == rhs.alphabet && states == rhs.states &&
              start_state == rhs.start_state &&
              final_states == rhs.final_states);
    }

    bool contain_final_state(const std::set<state_type> &T) const {
      for (const auto &f : final_states) {
        if (T.count(f) == 1) {
          return true;
        }
      }
      return false;
    }

    bool is_final_state(state_type final_state) const {
      return final_states.count(final_state) > 0;
    }

    bool includes(const std::set<state_type> &T) const {
      return ranges::v3::includes(states, T);
    }

  protected:
    std::shared_ptr<ALPHABET> alphabet;
    std::set<state_type> states;
    state_type start_state;
    std::set<state_type> final_states;
  };

} // namespace cyy::computation
