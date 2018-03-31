/*!
 * \file cfg_test.cpp
 *
 * \brief 测试cfg
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <iostream>

#include "../src/automaton.hpp"

using namespace cyy::lang;
TEST_CASE("simulate DFA") {

  DFA dfa({0,1,2,3},"ASCII",0,{

      { {0,'a'},1 }, 
      { {0,'b'},0 }, 

      { {1,'a'},1 }, 
      { {1,'b'},2 }, 
      { {2,'a'},1 }, 
      { {2,'b'},3 }, 
      { {3,'a'},1 }, 
      { {3,'b'},0 }, 
      },{3});

  symbol_string str={'a','b','b'};

  CHECK(dfa.simulate(str));
  str.insert(str.begin(),'a');

  CHECK(dfa.simulate(str));
  str.insert(str.begin(),'b');

  CHECK(dfa.simulate(str));
  str.pop_back();
  CHECK(!dfa.simulate(str));

  }

