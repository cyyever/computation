/*!
 * \file cnf.cpp
 *
 * \brief
 */

#include "cnf.hpp"

#include <cassert>
#include <vector>

namespace cyy::computation {
  bool CNF::valid() const {
    for (auto &[head, bodies] : get_productions()) {
      for (auto &body : bodies) {
        auto body_size = body.size();
        if (body_size > 2) {
          return false;
        }
        if (body_size == 0) {
          if (head != get_start_symbol()) {
            return false;
          }
        }
        if (body_size == 1) {
          if (!body[0].is_terminal()) {
            return false;
          }
        }
        if (body_size == 2) {
          if (std::any_of(body.begin(), body.end(), [*this](auto const &g) {
                return g.is_terminal() || g == get_start_symbol();
              })) {
            return false;
          }
        }
      }
    }
    return true;
  }
  bool CNF::parse(symbol_string_view view) const {
    make_reverse_productions();
    if (view.empty()) {
      return reverse_productions[{}].contains(get_start_symbol());
    }
    std::vector<std::vector<std::set<nonterminal_type>>> table(view.size());
    for (size_t i = 0; i < view.size(); i++) {
      table[i].resize(view.size());
      table[i][i] = reverse_productions[{view[i]}];
    }

    for (size_t len = 2; len <= view.size(); len++) {
      for (size_t i = 0; i + len - 1 < view.size(); i++) {
        auto j = i + len - 1;
        for (size_t k = i; k < j; k++) {
          auto const &first_heads = table[i][k];
          auto const &second_heads = table[k + 1][j];
          for (auto const &A : first_heads) {
            for (auto const &B : second_heads) {
              table[i][j].merge(
                  std::set<nonterminal_type>(reverse_productions[{A, B}]));
            }
          }
        }
      }
    }
    return table[0][view.size() - 1].contains(get_start_symbol());
  }
  void CNF::make_reverse_productions() const {
    if (!reverse_productions.empty()) {
      return;
    }
    for (auto const &[head, bodies] : get_productions()) {
      for (auto const &body : bodies) {
        reverse_productions[body].emplace(head);
      }
    }
  }

} // namespace cyy::computation
