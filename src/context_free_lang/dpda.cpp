/*!
 * \file dpda.cpp
 *
 * \author cyy
 * \date 2018-03-03
 */

#include "dpda.hpp"

#include <cassert>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "lang/endmarked_alphabet.hpp"
#include "lang/union_alphabet.hpp"

namespace cyy::computation {
  bool DPDA::recognize(symbol_string_view view) const {
    configuration_type configuration{get_start_state(), {}};

    size_t i = 0;
    while (i < view.size()) {
      auto const &symbol = view[i];
      auto configuration_opt = go(configuration, symbol);
      if (configuration_opt) {
        configuration = std::move(*configuration_opt);
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
      configuration = std::move(*configuration_opt);
    }
    return true;
  }

  std::optional<DPDA::configuration_type>
  DPDA::go(configuration_type configuration) const {
    auto state = configuration.state;

    auto it = transition_function.find(state);
    if (it == transition_function.end()) {
      throw exception::unexisted_transition(std::to_string(state));
    }
    auto const &state_transition_function = it->second;

    auto it2 = state_transition_function.find({});
    if (it2 != state_transition_function.end()) {
      configuration.state = it2->second.state;
      if (it2->second.has_push()) {
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
      if (it2->second.has_push()) {
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
      if (it2->second.has_push()) {
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
      if (it2->second.has_push()) {
        configuration.stack.push_back(it2->second.stack_symbol.value());
      }
      return configuration;
    }
    return {};
  }
  void DPDA::normalize() {
    if (has_normalized) {
      return;
    }
    if (stack_alphabet->contain(ALPHABET::endmarker)) {
      throw std::logic_error(
          "can't normalize DPDA since it has endmarker in stack");
    }
    auto [acceptance_looping_situations, rejection_looping_situations] =
        get_looping_situations();
    std::unordered_map<state_type, state_type> parallel_states;

    transition_function_type new_transitions;
    auto create_parallel_state = [&, this](auto &&self, state_type s) {
      if (parallel_states.contains(s)) {
        return parallel_states[s];
      }
      auto parallel_state = add_new_state();
      parallel_states[s] = parallel_state;

      for (auto [situation, action] : transition_function[s]) {
        if (!situation.use_input()) {
          action.state = self(self, action.state);
        }
        new_transitions[parallel_state].emplace(situation, action);
      }
      return parallel_state;
    };

    for (auto final_state : final_states) {
      for (auto &[situation, action] : transition_function[final_state]) {
        if (!situation.use_input()) {
          action.state =
              create_parallel_state(create_parallel_state, action.state);
        }
      }
    }

    for (auto const &[_, parallel_state] : parallel_states) {
      add_final_state(parallel_state);
    }

    transition_function.merge(std::move(new_transitions));
    check_transition_fuction();

    auto old_start_state = get_start_state();
    change_start_state(add_new_state());
    transition_function[get_start_state()][{}] = {old_start_state,
                                                  ALPHABET::endmarker};

    auto new_reject_state = add_new_state();
    auto new_accept_state = add_new_state();
    add_final_state(new_accept_state);

    for (auto &[from_state, transfers] : transition_function) {
      decltype(transfers) new_transfers;
      bool flag = false;
      for (const auto &[situation, action] : transfers) {
        if (!situation.has_pop()) {
          continue;
        }
        if (situation.use_input()) {
          continue;
        }
        auto new_situation = situation;
        new_situation.stack_symbol = ALPHABET::endmarker;
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
          if (!situation.has_pop()) {
            continue;
          }
          auto new_situation = situation;
          new_situation.stack_symbol = ALPHABET::endmarker;
          new_transfers[new_situation] = {new_reject_state};
        }
      }
      transfers.merge(std::move(new_transfers));
    }

    transition_function.make_reject_state(new_reject_state, alphabet);
    for (auto a : alphabet->get_view()) {
      transition_function[new_accept_state][{a}] = {new_reject_state};
    }

    for (auto &[state, stack_symbols] : acceptance_looping_situations) {
      if (transition_function[state].contains({})) {
        assert(stack_symbols.size() == stack_alphabet->size());
        transition_function[state][{}] = {new_accept_state};
        continue;
      }

      for (auto const stack_symbol : stack_symbols) {
        transition_function[state][{{}, stack_symbol}] = {new_accept_state};
      }
    }
    for (auto const &[state, stack_symbols] : rejection_looping_situations) {
      if (transition_function[state].contains({})) {
        assert(stack_symbols.size() == stack_alphabet->size());
        transition_function[state][{}] = {new_reject_state};
        continue;
      }

      for (auto const stack_symbol : stack_symbols) {
        transition_function[state][{{}, stack_symbol}] = {new_reject_state};
      }
    }
    has_normalized = true;
    reject_state_opt = new_reject_state;

    stack_alphabet = std::make_shared<endmarked_alphabet>(stack_alphabet);
#ifndef NDEBUG
    check_transition_fuction();
#endif
  }

  std::pair<std::map<DPDA::state_type, symbol_set_type>,
            std::map<DPDA::state_type, symbol_set_type>>
  DPDA::get_looping_situations() const {
    std::map<state_type, std::set<std::pair<stack_symbol_type, bool>>>
        looping_situations;

    state_set_map_type epsilon_transitions;
    for (const auto &[from_state, transfers] : transition_function) {
      for (const auto &[situation, action] : transfers) {
        if (situation.use_input()) {
          continue;
        }
        epsilon_transitions[from_state].insert(action.state);
        auto can_reach_final = is_final_state(action.state);

        if (situation.has_pop()) {
          looping_situations[from_state].emplace(situation.get_poped_symbol(),
                                                 can_reach_final);
          continue;
        }
        for (auto stack_symbol : stack_alphabet->get_view()) {
          looping_situations[from_state].emplace(stack_symbol, can_reach_final);
        }
      }
    }

    while (true) {
      bool flag = false;
      auto new_looping_situations = looping_situations;
      for (auto &[looping_state, stack_symbol_set] : looping_situations) {
        auto it = transition_function.find(looping_state);
        auto const &state_transition_function = it->second;
        auto it2 = state_transition_function.find({});
        if (it2 != state_transition_function.end()) {
          auto const &action = it2->second;
          if (!new_looping_situations.contains(action.state)) {
            if (new_looping_situations.erase(looping_state)) {
              flag = true;
            }
            continue;
          }

          if (!action.has_push()) {
            if (new_looping_situations[looping_state] !=
                new_looping_situations[action.state]) {
              new_looping_situations[looping_state] =
                  new_looping_situations[action.state];
              flag = true;
            }
            continue;
          }

          auto it3 = new_looping_situations[action.state].find(
              {action.get_pushed_symbol(), false});
          if (it3 == new_looping_situations[action.state].end()) {
            it3 = new_looping_situations[action.state].find(
                {action.get_pushed_symbol(), true});
          }
          if (it3 == new_looping_situations[action.state].end()) {
            if (new_looping_situations.erase(looping_state)) {
              flag = true;
            }
          } else {
            // reach to final
            if (it3->second) {
              std::set<std::pair<stack_symbol_type, bool>> new_set;
              for (auto tmp : new_looping_situations[looping_state]) {
                if (!tmp.second) {
                  tmp.second = true;
                  flag = true;
                }
                new_set.emplace(std::move(tmp));
              }
              new_looping_situations[looping_state] = std::move(new_set);
            }
          }
          continue;
        }

        for (auto stack_symbol_and_state : stack_symbol_set) {
          auto [stack_symbol, can_reach_final] = stack_symbol_and_state;
          it2 = state_transition_function.find({{}, stack_symbol});
          if (it2 == state_transition_function.end()) {
            continue;
          }
          auto const &action = it2->second;

          if (!new_looping_situations.contains(action.state)) {
            if (new_looping_situations[looping_state].erase(
                    stack_symbol_and_state)) {
              flag = true;
            }
            continue;
          }

          if (action.has_push()) {

            auto it3 = new_looping_situations[action.state].find(
                {action.get_pushed_symbol(), false});
            if (it3 == new_looping_situations[action.state].end()) {
              it3 = new_looping_situations[action.state].find(
                  {action.get_pushed_symbol(), true});
            }

            if (it3 == new_looping_situations[action.state].end()) {
              if (new_looping_situations[looping_state].erase(
                      stack_symbol_and_state)) {
                flag = true;
              }
              continue;
            } else if (!can_reach_final) {
              if (it3->second) {
                new_looping_situations[looping_state].emplace(stack_symbol,
                                                              true);
                flag = true;
              }
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

    // FIXME epsilon_transitions
    std::map<state_type, symbol_set_type> acceptance_looping_situations,
        rejection_looping_situations;
    for (auto &[s, stack_symbol_set] : looping_situations) {
      assert(!stack_symbol_set.empty());
      for (auto [stack_symbol, can_reach_final] : stack_symbol_set) {
        if (can_reach_final) {
          acceptance_looping_situations[s].emplace(stack_symbol);
        } else {
          rejection_looping_situations[s].emplace(stack_symbol);
        }
      }
    }
    return {acceptance_looping_situations, rejection_looping_situations};
  }
  DPDA DPDA::complement() const {
    auto complement_dpda = *this;
    complement_dpda.normalize();

    if (complement_dpda.final_states.empty()) {
      complement_dpda.mark_all_states_final();
      return complement_dpda;
    }
    state_set_type reading_states;
    // mark reading states
    transition_function_type new_transitions;
    for (auto &[from_state, transfers] : complement_dpda.transition_function) {
      bool has_input_epsilon = std::ranges::any_of(
          transfers, [](auto const p) { return !p.first.use_input(); });
      bool has_input = std::ranges::any_of(
          transfers, [](auto const p) { return p.first.use_input(); });
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
        if (situation.use_input()) {
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

      auto count = std::erase_if(transfers, [](const auto &transfer) {
        return transfer.first.use_input();
      });
      assert(count > 0);
      transfers.merge(std::move(new_transitions_of_state));
    }
    complement_dpda.transition_function.merge(std::move(new_transitions));

#ifndef NDEBUG
    for (auto const s : reading_states) {
      assert(std::ranges::all_of(
          complement_dpda.transition_function[s],
          [](auto const p) { return p.first.use_input(); }));
    }
#endif
    state_set_type new_final_states;
    std::ranges::set_difference(
        reading_states, complement_dpda.final_states,
        std::inserter(new_final_states, new_final_states.begin()));
    complement_dpda.final_states = std::move(new_final_states);

#ifndef NDEBUG
    complement_dpda.check_transition_fuction();
#endif

    return complement_dpda;
  }

  void DPDA::check_transition_fuction() const {
#ifndef NDEBUG
    for (auto state : get_states()) {
      auto it = transition_function.find(state);
      if (it == transition_function.end()) {
        throw exception::no_DPDA(std::string("lack transitions for state ") +
                                 std::to_string(state));
      }
      auto const &state_transition_function = it->second;
      for (auto input_symbol : alphabet->get_view()) {
        for (auto stack_symbol : stack_alphabet->get_view()) {
          size_t cnt = 0;
          cnt += state_transition_function.count({});
          cnt += state_transition_function.count({input_symbol});
          cnt += state_transition_function.count({{}, stack_symbol});
          cnt += state_transition_function.count({input_symbol, stack_symbol});

          if (cnt == 0) {
            throw exception::no_DPDA(
                std::string("the combinations of the state ") +
                std::to_string(state) + " and input symbol " +
                alphabet->to_string(input_symbol) + " and stack symbol " +
                stack_alphabet->to_string(stack_symbol) + " lead to no branch");
          }
          if (cnt > 1) {
            throw exception::no_DPDA(
                std::string("the combinations of the state ") +
                std::to_string(state) + " and input symbol " +
                alphabet->to_string(input_symbol) + " and stack symbol " +
                stack_alphabet->to_string(stack_symbol) +
                " lead to multiple branches");
          }
        }
      }
    }
#endif
  }
  std::string DPDA::MMA_draw() const {
    std::map<
        std::tuple<state_type, state_type, std::optional<input_symbol_type>>,
        std::vector<std::string>>
        stack_cmds;
    for (auto &[from_state, transfers] : transition_function) {
      for (const auto &[situation, action] : transfers) {
        std::string sub_cmd = "{";
        if (situation.has_pop()) {
          sub_cmd += stack_alphabet->MMA_draw(situation.stack_symbol.value());
        } else {
          sub_cmd += "\\[Epsilon]";
        }
        sub_cmd += ",\"/\",";
        if (action.has_push()) {
          sub_cmd += stack_alphabet->MMA_draw(action.stack_symbol.value());
        } else {
          sub_cmd += "\\[Epsilon]";
        }
        sub_cmd += "}";
        stack_cmds[{from_state, action.state, situation.input_symbol}]
            .emplace_back(std::move(sub_cmd));
      }
    }
    std::map<std::tuple<state_type, state_type>, std::string> edge_cmds;
    for (auto &[k, v] : stack_cmds) {
      auto const &[from_state, to_state, input_symbol] = k;
      std::string sub_cmd = "{";
      if (input_symbol.has_value()) {
        sub_cmd += alphabet->MMA_draw(input_symbol.value());
      } else {
        sub_cmd += "\\[Epsilon]";
      }
      sub_cmd += ",{";
      for (auto stack_cmd : v) {
        sub_cmd += stack_cmd;
        sub_cmd.push_back(',');
      }
      sub_cmd.pop_back();
      sub_cmd += "}}";
      edge_cmds[{from_state, to_state}] += "," + sub_cmd;
    }

    std::stringstream is;
    is << "Graph[{";
    for (auto &[k, v] : edge_cmds) {
      auto const &[from_state, to_state] = k;
      is << "Labeled[ " << from_state << "->" << to_state << ",{" << v.substr(1)
         << '}' << "],";
    }
    // drop last ,
    is.seekp(-1, std::ios_base::end);
    is << "}," << finite_automaton::MMA_draw() << ']';
    return is.str();
  }
} // namespace cyy::computation
