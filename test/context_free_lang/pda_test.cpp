/*!
 * \file pda_test.cpp
 *
 * \brief 测试pda
 */
#if __has_include(<CppCoreCheck\Warnings.h>)
#include <CppCoreCheck\Warnings.h>
#pragma warning(disable : ALL_CPPCORECHECK_WARNINGS)
#endif
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "../../src/context_free_lang/pda.hpp"
#include "../../src/lang/alphabet.hpp"
#include "../../src/lang/set_alphabet.hpp"

using namespace cyy::computation;
TEST_CASE("simulate PDA") {

  auto input_alphabet =
      std::make_shared<set_alphabet>(std::set<symbol_type>{'0', '1'}, "01_set");
  ALPHABET::set(input_alphabet);

  PDA pda({0, 1, 2, 3}, "01_set", "01_set", 0,
          {
              {{std::optional<PDA::input_symbol_type>{}, 0, {}},
               {{1, input_alphabet->get_endmarker()}}},
              {{U'0', 1, {}}, {{1, U'0'}}},
              {{U'1', 1, {}}, {{1, U'1'}}},
              {{std::optional<PDA::input_symbol_type>{}, 1, {}}, {{2, {}}}},
              {{U'0', 2, U'0'}, {{2, {}}}},
              {{U'1', 2, U'1'}, {{2, {}}}},
              {{std::optional<PDA::input_symbol_type>{}, 2,
                input_alphabet->get_endmarker()},
               {{3, {}}}},
          },
          {3});

  SUBCASE("0") {
    symbol_string str = U"0";
    CHECK(!pda.simulate(str));
  }

  SUBCASE("01") {
    symbol_string str = U"01";
    CHECK(!pda.simulate(str));
  }
  SUBCASE("010") {
    symbol_string str = U"010";
    CHECK(!pda.simulate(str));
  }
  SUBCASE("0101") {
    symbol_string str = U"0101";
    CHECK(!pda.simulate(str));
  }
  SUBCASE("0110") {
    symbol_string str = U"0110";
    CHECK(pda.simulate(str));
  }
  SUBCASE("1001") {
    symbol_string str = U"1001";
    CHECK(pda.simulate(str));
  }
  SUBCASE("normalize") {
    pda.normalize_transitions();
    symbol_string str = U"1001";
    CHECK(pda.simulate(str));
    str = U"0101";
    CHECK(!pda.simulate(str));
  }
}
