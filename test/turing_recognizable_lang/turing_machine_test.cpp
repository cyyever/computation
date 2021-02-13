/*!
 * \file cfg_test.cpp
 *
 * \brief 测试dfa
 */
#include <ranges>

#include <doctest/doctest.h>

#include "lang/number_set_alphabet.hpp"
#include "turing_recognizable_lang/turing_machine.hpp"

using namespace cyy::computation;
TEST_CASE("recognize Turing machine") {
  /* auto alphabet=std::make_shared<symbol_set_type>({'0','1','#'},"01#_set");
   */

  finite_automaton automaton({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, "01#_set", 1, {0});

  Turing_machine::transition_function_type transition_function;
  transition_function.add_moving_right_transitions(1, {'#'}, 8);
  transition_function.add_moving_right_transition(1, '0', 2, 'x');
  transition_function.add_moving_right_transition(1, '1', 3, 'x');
  transition_function.add_moving_right_transitions(2, {'0', '1'});
  transition_function.add_moving_right_transitions(2, {'#'}, 4);
  transition_function.add_moving_right_transitions(4, {'x'});
  transition_function.add_moving_left_transition(4, '0', 6, 'x');
  transition_function.add_moving_right_transitions(3, {'0', '1'});
  transition_function.add_moving_right_transitions(3, {'#'}, 5);
  transition_function.add_moving_right_transitions(5, {'x'});
  transition_function.add_moving_left_transition(5, '1', 6, 'x');
  transition_function.add_moving_left_transitions(6, {'x', '1', '0'});
  transition_function.add_moving_left_transitions(6, {'#'}, 7);
  transition_function.add_moving_left_transitions(7, {'0', '1'});
  transition_function.add_moving_right_transitions(7, {'x'}, 1);
  transition_function.add_moving_right_transitions(8, {'x'});
  transition_function.add_moving_right_transitions(8, {ALPHABET::blank_symbol},
                                                   0);

  Turing_machine tm(automaton, 9, "01x#_set", transition_function);

  SUBCASE("") { CHECK(!tm.recognize(U"")); }
  SUBCASE("0#1") { CHECK(!tm.recognize(U"0#1")); }
  SUBCASE("1#0") { CHECK(!tm.recognize(U"1#0")); }
  SUBCASE("#") { CHECK(tm.recognize(U"#")); }
  SUBCASE("0#0") { CHECK(tm.recognize(U"0#0")); }
  SUBCASE("1#1") { CHECK(tm.recognize(U"1#1")); }
  SUBCASE("00#00") { CHECK(tm.recognize(U"00#00")); }
  SUBCASE("11#11") { CHECK(tm.recognize(U"11#11")); }
}
