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
TEST_CASE("recognize PDA") {

  auto input_alphabet =
      std::make_shared<set_alphabet>(std::set<symbol_type>{'0', '1'}, "01_set");
  ALPHABET::set(input_alphabet);

  auto endmarker = ALPHABET::endmarker;
  finite_automata pda_automata({0, 1, 2, 3}, "01_set", 0, {3});
  PDA pda(std::move(pda_automata), "01_set",
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
          });

  SUBCASE("0") { CHECK(!pda.recognize(U"0")); }

  SUBCASE("01") { CHECK(!pda.recognize(U"01")); }
  SUBCASE("010") { CHECK(!pda.recognize(U"010")); }
  SUBCASE("0101") { CHECK(!pda.recognize(U"0101")); }
  SUBCASE("0110") { CHECK(pda.recognize(U"0110")); }
  SUBCASE("1001") { CHECK(pda.recognize(U"1001")); }
  SUBCASE("prepare_CFG_conversion") {
    pda.prepare_CFG_conversion();
    symbol_string str = U"1001";
    CHECK(pda.recognize(str));
    str = U"0101";
    CHECK(!pda.recognize(str));
  }
  SUBCASE("to_CFG") {
    auto cfg = PDA_to_CFG(PDA(pda));
    cfg.to_CNF();
    CNF cnf(cfg);

    SUBCASE("0") { CHECK(!cnf.parse(U"0")); }

    SUBCASE("01") { CHECK(!cnf.parse(U"01")); }
    SUBCASE("010") { CHECK(!cnf.parse(U"010")); }
    SUBCASE("0101") { CHECK(!cnf.parse(U"0101")); }
    SUBCASE("0110") { CHECK(cnf.parse(U"0110")); }
    SUBCASE("1001") { CHECK(cnf.parse(U"1001")); }
  }
  SUBCASE("draw") { std::cout << pda.MMA_draw() << std::endl; }
}
