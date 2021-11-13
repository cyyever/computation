/*!
 * \file helper.cpp
 *
 * \brief fuzzing helper functions
 * \author cyy
 * \date 2019-02-12
 */
#include "helper.hpp"

using namespace cyy::computation;
CFG::production_set_type fuzzing_CFG_productions(const uint8_t *Data,
                                                 size_t Size) {
  CFG::production_set_type productions;

  size_t i = 0;
  while (i < Size) {
    CFG::nonterminal_type header;
    header = CFG::nonterminal_type(1, to_printable_ASCII(Data[i]));
    i++;

    if (i < Size) {
      CFG_production::body_type body;
      size_t body_size = Data[i];
      i++;
      for (size_t j = 0; j < body_size && i < Size; j++, i += 2) {
        if (i + 1 < Size) {
          if (Data[i] % 2 == 0) {
            body.emplace_back(grammar_symbol_type::nonterminal_type(
                1, to_printable_ASCII(Data[i + 1])));
          } else {
            body.push_back(grammar_symbol_type::terminal_type(
                to_printable_ASCII(Data[i + 1])));
          }
        }
      }
      productions[header].emplace(std::move(body));
    }
  }
  return productions;
}

NFA fuzzing_NFA(const uint8_t *Data, size_t Size) {
  NFA::state_set_type states;
  NFA::state_type start_state = 0;
  NFA::transition_function_type transition_function;
  NFA::epsilon_transition_function_type epsilon_transition_function;
    NFA::state_set_type final_states;
  auto alphabet = ALPHABET::get("common_tokens");
  size_t i = 0;
  if (i < Size) {
    auto state_num = Data[i];
    i++;
    while (i < Size && i < state_num) {
      states.insert(Data[i]);
      i++;
    }
  }
  if (i < Size) {
    auto state_num = Data[i];
    i++;
    while (i < Size && i < state_num) {
      final_states.insert(Data[i]);
      i++;
    }
  }

  if (i < Size) {
    start_state = Data[i];
    i++;
  }

  while (i < Size) {
    auto from_state = Data[i];
    i++;

    std::set<NFA::state_type> to_states;
    if (i < Size) {
      auto state_num = Data[i];
      i++;
      while (i < Size && i < state_num) {
        to_states.insert(Data[i]);
        i++;
      }
    }

    if (i < Size) {
      for (auto s : alphabet->get_view()) {
        if (s >= Data[i]) {
          transition_function.try_emplace({s, from_state}, to_states);
        }
      }
      i++;
    }
    if (i < Size) {
      epsilon_transition_function.try_emplace(from_state, to_states);
      i++;
    }
  }
  return NFA(states, alphabet, start_state, transition_function,
             final_states);
}

symbol_string fuzzing_symbol_string(const uint8_t *Data, size_t Size) {
  symbol_string str;
  for (size_t i = 0; i < Size; i++) {
    str.push_back(static_cast<symbol_type>(to_printable_ASCII(Data[i])));
  }
  return str;
}
