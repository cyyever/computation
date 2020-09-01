
#include "automata.hpp"
#include "../util.hpp"
#include <sstream>
namespace cyy::computation {

  finite_automata::state_set_type &finite_automata::get_epsilon_closure(
      state_set_map_type &epsilon_closures, state_type s,
      const state_set_map_type &epsilon_transition_function) {

    auto it = epsilon_closures.find(s);
    if (it != epsilon_closures.end()) {
      return it->second;
    }

    std::unordered_map<state_type, state_set_type> dependency;
    state_set_type unstable_states{s};
    std::vector<state_type> stack{s};
    for (size_t i = 0; i < stack.size(); i++) {
      auto unstable_state = stack[i];
      auto it2 = epsilon_transition_function.find(unstable_state);
      if (it2 == epsilon_transition_function.end()) {
        continue;
      }

      for (auto next_state : it2->second) {
        auto it3 = epsilon_closures.find(next_state);
        if (it3 != epsilon_closures.end()) {
          epsilon_closures[unstable_state].merge(state_set_type(it3->second));
        } else {
          if (unstable_states.insert(next_state).second) {
            stack.push_back(next_state);
          }
          dependency[next_state].insert(unstable_state);
        }
      }
    }

    for (auto unstable_state : unstable_states) {
      epsilon_closures[unstable_state].insert(unstable_state);
    }

    auto [sorted_states, remain_dependency] = topological_sort(dependency);

    for (auto sorted_state : sorted_states) {
      for (auto prev_state : dependency[sorted_state]) {
        state_set_type diff;
        auto &prev_epsilon_closure = epsilon_closures[prev_state];
        auto &unstable_epsilon_closure = epsilon_closures[sorted_state];
        std::ranges::set_difference(unstable_epsilon_closure,
                                    prev_epsilon_closure,
                                    std::inserter(diff, diff.begin()));

        if (!diff.empty()) {
          prev_epsilon_closure.merge(std::move(diff));
        }
      }
      unstable_states.erase(sorted_state);
    }

    while (!unstable_states.empty()) {
      auto it2 = unstable_states.begin();
      auto unstable_state = *it2;
      unstable_states.erase(it2);
      for (auto prev_state : remain_dependency[unstable_state]) {
        state_set_type diff;
        auto &prev_epsilon_closure = epsilon_closures[prev_state];
        auto &unstable_epsilon_closure = epsilon_closures[unstable_state];
        std::ranges::set_difference(unstable_epsilon_closure,
                                    prev_epsilon_closure,
                                    std::inserter(diff, diff.begin()));

        if (!diff.empty()) {
          prev_epsilon_closure.merge(std::move(diff));
          unstable_states.insert(prev_state);
        }
      }
    }
    return epsilon_closures[s];
  }

  finite_automata::state_bitset_type
  finite_automata::get_bitset(uint64_t bitset_value) const {
    return state_bitset_type(states.size(), bitset_value);
  }

  finite_automata::state_bitset_type
  finite_automata::get_bitset(const state_set_type &state_set) const {
    state_bitset_type bitset(states.size());
    auto it = states.begin();
    auto it2 = state_set.begin();
    while (it != states.end() && it2 != state_set.end()) {
      if (*it == *it2) {
        bitset.set(std::distance(states.begin(), it));
        it++;
        it2++;
        continue;
      }
      if (*it < *it2) {
        it++;
        continue;
      }
      it2++;
    }
    return bitset;
  }

  finite_automata::state_set_type
  finite_automata::from_bitset(const state_bitset_type &bitset) const {
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
  finite_automata::state_bitset_contains(const state_bitset_type &state_bitset,
                                         state_type state) const {
    return state_bitset.test(std::distance(states.begin(), states.find(state)));
  }

  std::string finite_automata::MMA_draw() const {
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
} // namespace cyy::computation
