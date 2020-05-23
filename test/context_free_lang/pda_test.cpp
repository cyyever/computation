/*!
 * \file pda_test.cpp
 *
 * \brief 测试pda
 */
#include <doctest/doctest.h>

#include "../../src/context_free_lang/cnf.hpp"
#include "../../src/context_free_lang/model_transform.hpp"
#include "../../src/context_free_lang/pda.hpp"
#include "../../src/lang/alphabet.hpp"
#include "../../src/lang/set_alphabet.hpp"

using namespace cyy::computation;
TEST_CASE("simulate PDA") {

  auto input_alphabet =
      std::make_shared<set_alphabet>(std::set<symbol_type>{'0', '1'}, "01_set");
  ALPHABET::set(input_alphabet);

  auto endmarker = input_alphabet->get_endmarker();
  PDA pda({0, 1, 2, 3}, "01_set", "01_set", 0,
          {
              {{0}, {{1, endmarker}}},
              {{
                   1,
                   U'0',
               },
               {{1, U'0'}}},
              {{
                   1,
                   U'1',
               },
               {{1, U'1'}}},
              {{1}, {{2, {}}}},
              {{2, U'0', U'0'}, {{2, {}}}},
              {{2, U'1', U'1'}, {{2, {}}}},
              {{2, {}, endmarker}, {{3, {}}}},
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
  SUBCASE("to_CFG") {
    auto cfg = PDA_to_CFG(PDA(pda));
    cfg.to_CNF();
    CNF cnf(cfg);
    symbol_string str = U"1001";
    CHECK(cnf.parse(str));
    str = U"0101";
    CHECK(!cnf.parse(str));
  }
}
