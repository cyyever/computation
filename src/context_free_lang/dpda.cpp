/*!
 * \file dpda.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include <algorithm>
#include <cassert>
#include <iterator>
#include <unordered_map>
#include <vector>

#include "dpda.hpp"

namespace cyy::computation {
  bool DPDA::simulate(symbol_string_view view) const {
    configuration_type configuration{start_state, {}};

    size_t i = 0;
    while (i < view.size()) {
      auto const &symbol = view[i];
      auto configuration_opt = go(configuration, symbol);
      if (configuration_opt) {
        configuration = std::move(configuration_opt.value());
        i++;
        continue;
      }
      configuration_opt = go(std::move(configuration));
      assert(configuration_opt.has_value());
      configuration = std::move(configuration_opt.value());
    }
    while (!is_final_state(configuration.state)) {
      auto configuration_opt = go(std::move(configuration));
      if (!configuration_opt) {
        return false;
      }
      configuration = std::move(configuration_opt.value());
    }
    return true;
  }

  std::optional<DPDA::configuration_type>
  DPDA::go(configuration_type configuration) const {
    auto state = configuration.state;

    auto it = transition_function.find(state);
    if (it == transition_function.end()) {
      return {};
    }
    auto const &state_transition_function = it->second;

    auto it2 = state_transition_function.find({});
    if (it2 != state_transition_function.end()) {
      configuration.state = it2->second.state;
      if (it2->second.stack_symbol.has_value()) {
        configuration.stack.push_back(it2->second.stack_symbol.value());
      }
      return configuration;
    }
    if (configuration.stack.empty()) {
      return {};
    }
    auto stack_top = configuration.stack.back();

    it2 = state_transition_function.find({{}, stack_top});
    if (it2 != state_transition_function.end()) {
      configuration.state = it2->second.state;
      configuration.stack.pop_back();
      if (it2->second.stack_symbol.has_value()) {
        configuration.stack.push_back(it2->second.stack_symbol.value());
      }
      return configuration;
    }
    return {};
  }

  std::optional<DPDA::configuration_type>
  DPDA::go(configuration_type configuration, input_symbol_type a) const {
    auto state = configuration.state;
    auto it = transition_function.find(state);
    if (it == transition_function.end()) {
      return {};
    }
    auto const &state_transition_function = it->second;
    auto it2 = state_transition_function.find({a});
    if (it2 != state_transition_function.end()) {
      configuration.state = it2->second.state;
      if (it2->second.stack_symbol.has_value()) {
        configuration.stack.push_back(it2->second.stack_symbol.value());
      }
      return configuration;
    }
    if (configuration.stack.empty()) {
      return {};
    }
    auto stack_top = configuration.stack.back();

    it2 = state_transition_function.find({a, stack_top});
    if (it2 != state_transition_function.end()) {
      configuration.state = it2->second.state;
      configuration.stack.pop_back();
      if (it2->second.stack_symbol.has_value()) {
        configuration.stack.push_back(it2->second.stack_symbol.value());
      }
      return configuration;
    }
    return {};
  }
  void DPDA::normalize() {
    auto [acceptance_looping_situations, rejection_looping_situations] =
        get_looping_situations();
    std::unordered_map<state_type, state_type> parallel_states;
    auto get_parallel_state = [&, this](state_type s) {
      auto it = parallel_states.find(s);
      if (it == parallel_states.end()) {
        auto parallel_state = add_new_state();
        add_final_states(parallel_state);
        it = parallel_states.emplace(s, parallel_state).first;
        assert(it->second == parallel_state);
      }
      return it->second;
    };
    transition_function_type new_transitions;
    for (auto &[from_state, transfers] : transition_function) {
      auto parallel_from_state = get_parallel_state(from_state);
      for (auto &[configuration, action] : transfers) {
        if (configuration.input_symbol.has_value()) {
          new_transitions[parallel_from_state].emplace(configuration, action);
        } else {
          auto new_action = action;
          new_action.state = get_parallel_state(action.state);
          new_transitions[parallel_from_state].emplace(configuration,
                                                       new_action);
          if (is_final_state(from_state)) {
            action.state = get_parallel_state(action.state);
          }
        }
      }
      assert(new_transitions.contains(parallel_from_state));
    }

    transition_function.merge(std::move(new_transitions));

    auto old_start_state = start_state;
    auto endmarker = stack_alphabet->get_endmarker();
    start_state = add_new_state();
    transition_function[start_state][{}] = {old_start_state, endmarker};

    auto new_reject_state = add_new_state();
    auto new_accept_state = add_new_state();
    add_final_states(new_accept_state);

    for (auto &[from_state, transfers] : transition_function) {
      decltype(transfers) new_transfers;
      bool flag = false;
      for (const auto &[situation, action] : transfers) {
        if (!situation.stack_symbol.has_value()) {
          continue;
        }
        if (situation.input_symbol.has_value()) {
          continue;
        }
        auto new_situation = situation;
        new_situation.stack_symbol = endmarker;
        if (is_final_state(from_state)) {
          new_transfers[new_situation] = {new_accept_state};
        } else {
          new_transfers[new_situation] = {new_reject_state};
        }
        flag = true;
        break;
      }
      if (!flag) {
        for (auto const &[situation, action] : transfers) {
          if (!situation.stack_symbol.has_value()) {
            continue;
          }
          auto new_situation = situation;
          new_situation.stack_symbol = endmarker;
          new_transfers[new_situation] = {new_reject_state};
        }
      }
      transfers.merge(std::move(new_transfers));
    }

    for (auto a : *alphabet) {
      transition_function[new_reject_state][{a}] = {new_reject_state};
      transition_function[new_accept_state][{a}] = {new_reject_state};
    }

    for (auto &[state, stack_symbols] : acceptance_looping_situations) {
      if (transition_function[state].contains({})) {
        assert(stack_symbols.size() == stack_alphabet->size());
        transition_function[state].clear();
        stack_symbols.insert(endmarker);
      }

      for (auto const stack_symbol : stack_symbols) {
        transition_function[state][{{}, stack_symbol}] = {new_accept_state};
      }
    }
    for (auto const &[state, stack_symbols] : rejection_looping_situations) {
      for (auto const stack_symbol : stack_symbols) {
        transition_function[state][{{}, stack_symbol}] = {new_reject_state};
      }
    }
#ifndef NDEBUG
    check_transition_fuction(true);
#endif
  }

  std::pair<std::map<DPDA::state_type, std::set<DPDA::stack_symbol_type>>,
            std::map<DPDA::state_type, std::set<DPDA::stack_symbol_type>>>
  DPDA::get_looping_situations() const {
    std::map<state_type, std::set<stack_symbol_type>> looping_situations;

    for (auto state : states) {
      auto &looping_situations_of_state = looping_situations[state];
      for (auto stack_symbol : *stack_alphabet) {
        looping_situations_of_state.insert(stack_symbol);
      }
    }

    state_set_map_type epsilon_transitions;

    for (const auto &[from_state, transfers] : transition_function) {
      for (const auto &[situation, action] : transfers) {
        if (situation.input_symbol.has_value()) {
          if (!situation.stack_symbol.has_value()) {
            looping_situations.erase(from_state);
            break;
          }
          looping_situations[from_state].erase(situation.stack_symbol.value());
        } else {
          epsilon_transitions[from_state].insert(action.state);
        }
      }
    }
    std::erase_if(looping_situations, [](const auto &item) {
      auto const &[_, value] = item;
      return value.empty();
    });

    while (true) {
      bool flag = false;
      auto new_looping_situations = looping_situations;
      for (auto &[from_state, stack_symbol_set] : looping_situations) {
        auto it = transition_function.find(from_state);
        auto const &state_transition_function = it->second;
        for (auto stack_symbol : stack_symbol_set) {
          action_type action;
          auto it2 = state_transition_function.find({{}, stack_symbol});
          if (it2 != state_transition_function.end()) {
            action = it2->second;
            if (!action.stack_symbol.has_value()) {
              if (new_looping_situations[from_state].erase(stack_symbol)) {
                flag = true;
              }
              continue;
            }
          } else {
            it2 = state_transition_function.find({});
            assert(it2 != state_transition_function.end());
            action = it2->second;

            if (!action.stack_symbol.has_value()) {
              auto prev_size = new_looping_situations[from_state].size();
              new_looping_situations[from_state] =
                  new_looping_situations[action.state];
              if (prev_size != new_looping_situations[from_state].size()) {
                flag = true;
                continue;
              }
            }
          }
          if (action.stack_symbol.has_value()) {
            if (!new_looping_situations.contains(action.state) ||
                !new_looping_situations[action.state].contains(
                    action.stack_symbol.value())) {
              if (new_looping_situations[from_state].erase(stack_symbol)) {
                flag = true;
              }
              continue;
            }
          }
        }
      }
      looping_situations = std::move(new_looping_situations);
      std::erase_if(looping_situations, [](const auto &item) {
        auto const &[_, value] = item;
        return value.empty();
      });
      if (!flag) {
        break;
      }
    }

    for (auto &[s, stack_symbol_set] : looping_situations) {
      assert(!stack_symbol_set.empty());
    }

    decltype(looping_situations) acceptance_looping_situations;
    decltype(looping_situations) rejection_looping_situations;
    for (auto &[s, stack_symbol_set] : looping_situations) {
      if (contain_final_state(get_epsilon_closure(s, epsilon_transitions))) {
        acceptance_looping_situations[s] = std::move(stack_symbol_set);
      } else {
        rejection_looping_situations[s] = std::move(stack_symbol_set);
      }
    }
    return {acceptance_looping_situations, rejection_looping_situations};
  }
  DPDA DPDA::complement() const {
    auto complement_dpda = *this;
    complement_dpda.normalize();

    if (complement_dpda.final_states.empty()) {
      complement_dpda.final_states = complement_dpda.states;
      return complement_dpda;
    }
    state_set_type reading_states;
    // mark reading states
    transition_function_type new_transitions;
    for (auto &[from_state, transfers] : complement_dpda.transition_function) {
      if (!transfers.begin()->first.stack_symbol.has_value()) {
        if (transfers.begin()->first.input_symbol.has_value()) {
          reading_states.insert(from_state);
        }
        continue;
      }
      bool has_input_epsilon = false;
      bool has_input = false;
      for (const auto &[situation, action] : transfers) {
        if (situation.input_symbol.has_value()) {
          has_input = true;
        } else {
          has_input_epsilon = true;
        }
      }
      if (!has_input_epsilon) {
        reading_states.insert(from_state);
        continue;
      }
      if (!has_input) {
        continue;
      }

      std::unordered_map<situation_type, action_type, situation_hash_type>
          new_transitions_of_state;
      std::unordered_map<stack_symbol_type, state_type> stack_to_state;
      for (auto [situation, action] : transfers) {
        if (situation.input_symbol.has_value()) {
          auto stack_symbol = situation.stack_symbol.value();
          auto it = stack_to_state.find(stack_symbol);
          if (it == stack_to_state.end()) {
            it = stack_to_state
                     .try_emplace(stack_symbol, complement_dpda.add_new_state())
                     .first;
            new_transitions_of_state[{{}, stack_symbol}] = {it->second};
            reading_states.insert(it->second);
            if (complement_dpda.is_final_state(from_state)) {
              complement_dpda.final_states.insert(it->second);
            }
          }

          new_transitions[it->second][{situation.input_symbol.value()}] =
              action;
        }
      }

      auto count = std::erase_if(transfers, [](const auto &item) {
        auto const &[key, value] = item;
        return key.input_symbol.has_value();
      });
      assert(count > 0);
      transfers.merge(std::move(new_transitions_of_state));
    }
    complement_dpda.transition_function.merge(std::move(new_transitions));

    assert(!reading_states.empty());
    state_set_type new_final_states;
    std::ranges::set_difference(
        reading_states, complement_dpda.final_states,
        std::inserter(new_final_states, new_final_states.begin()));
    assert(!new_final_states.empty());
    complement_dpda.final_states = std::move(new_final_states);

#ifndef NDEBUG
    complement_dpda.check_transition_fuction(true);
#endif

    return complement_dpda;
  }

  void DPDA::check_transition_fuction(bool check_endmark) {
    for (auto state : states) {
      auto it = transition_function.find(state);
      if (it == transition_function.end()) {
        throw exception::no_DPDA(std::string("lack transitions for state ") +
                                 std::to_string(state));
      }
      auto const &state_transition_function = it->second;
      for (auto input_symbol : *alphabet) {
        for (auto stack_symbol : stack_alphabet->get_view(check_endmark)) {
          size_t cnt = 0;
          cnt += state_transition_function.count({});
          cnt += state_transition_function.count({input_symbol});
          cnt += state_transition_function.count({{}, stack_symbol});
          cnt += state_transition_function.count({input_symbol, stack_symbol});
          if (cnt == 0) {
            throw exception::no_DPDA(
                std::string("the combinations of the state ") +
                std::to_string(state) + " and symbols " +
                alphabet->to_string(input_symbol) + " " +
                stack_alphabet->to_string(stack_symbol) + " lead to no branch");
          }
          if (cnt > 1) {
            throw exception::no_DPDA(
                std::string("the combinations of the state ") +
                std::to_string(state) + " and symbols " +
                alphabet->to_string(input_symbol) + " " +
                stack_alphabet->to_string(stack_symbol) +
                " lead to multiple branches");
          }
        }
      }
    }
  }
} // namespace cyy::computation
