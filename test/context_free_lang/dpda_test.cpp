/*!
 * \file pda_test.cpp
 *
 * \brief 测试dpda
 */
#include <doctest/doctest.h>

#include "../../src/context_free_lang/dpda.hpp"
#include "../../src/lang/alphabet.hpp"
#include "../../src/lang/set_alphabet.hpp"

using namespace cyy::computation;
TEST_CASE("simulate DPDA") {

  auto input_alphabet =
      std::make_shared<set_alphabet>(std::set<symbol_type>{'0', '1'}, "01_set");
  ALPHABET::set(input_alphabet);

  auto endmarker = input_alphabet->get_endmarker();

  DPDA dpda(
      {0, 1, 2, 3}, "01_set", "01_set", 0,
      {

          {0,
           {
               {{std::optional<DPDA::input_symbol_type>{}, {}}, {1, endmarker}},
           }},
          {1,
           {
               {{U'0', {}}, {1, U'0'}},
               {{U'1', U'0'}, {2, {}}},
           }},
          {2,
           {
               {{U'1', U'0'}, {2, {}}},
               {{{}, endmarker}, {3, {}}},
           }}

      },
      {3});

  /* {{U'0', 1, {}}, {{1, U'0'}}}, */
  /* {{U'1', 1, {}}, {{1, U'1'}}}, */
  /* {{std::optional<DPDA::input_symbol_type>{}, 1, {}}, {{2, {}}}}, */
  /* {{U'0', 2, U'0'}, {{2, {}}}}, */
  /* {{U'1', 2, U'1'}, {{2, {}}}}, */
  /* {{std::optional<DPDA::input_symbol_type>{}, 2, */
  /*   input_alphabet->get_endmarker()}, */
  /*  {{3, {}}}}, */

  /* SUBCASE("0") { */
  /*   symbol_string str = U"0"; */
  /*   CHECK(!dpda.simulate(str)); */
  /* } */

  /* SUBCASE("01") { */
  /*   symbol_string str = U"01"; */
  /*   CHECK(!dpda.simulate(str)); */
  /* } */
  /* SUBCASE("010") { */
  /*   symbol_string str = U"010"; */
  /*   CHECK(!dpda.simulate(str)); */
  /* } */
  /* SUBCASE("0101") { */
  /*   symbol_string str = U"0101"; */
  /*   CHECK(!dpda.simulate(str)); */
  /* } */
  /* SUBCASE("0110") { */
  /*   symbol_string str = U"0110"; */
  /*   CHECK(dpda.simulate(str)); */
  /* } */
  /* SUBCASE("1001") { */
  /*   symbol_string str = U"1001"; */
  /*   CHECK(dpda.simulate(str)); */
  /* } */
}
