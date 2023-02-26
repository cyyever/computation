/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "cfg.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <utility>

#include <cyy/algorithm/alphabet/sub_alphabet.hpp>
#include <cyy/algorithm/alphabet/union_alphabet.hpp>

#include "exception.hpp"

namespace cyy::computation {
  using namespace cyy::algorithm;

  CFG::CFG(ALPHABET_ptr alphabet_, nonterminal_type start_symbol_,
           production_set_type productions_)
      : alphabet(std::move(alphabet_)), start_symbol(std::move(start_symbol_)),
        productions(std::move(productions_)) {

    eliminate_useless_symbols();
    normalize_productions();

    if (!productions.contains(start_symbol)) {
      throw exception::no_CFG("no productions for start symbol");
    }
    for (const auto &[head, bodies] : productions) {
      if (bodies.empty()) {
        throw exception::invalid_CFG_production(
            std::string("no body for head ") + head);
      }

      for (auto const &body : bodies) {
        for (auto const t : body.get_terminal_view()) {
          if (!alphabet->contain(t)) {
            throw exception::invalid_CFG_production(
                std::string("alphabet [") + alphabet->get_name() +
                "] does not contain terminal " + alphabet->to_string(t));
          }
        }
      }
    }
  }

  std::ostream &operator<<(std::ostream &os, const CFG &cfg) {
    // by convention,we print start symbol first.
    for (size_t i = 0; i < 2; i++) {
      for (auto const &[head, bodies] : cfg.productions) {
        if (i == 0 && head != cfg.start_symbol) {
          continue;
        }
        if (i == 1 && head == cfg.start_symbol) {
          continue;
        }
        for (auto const &body : bodies) {
          os << CFG_production(head, body).to_string(*cfg.alphabet);
        }
      }
    }
    return os;
  }

  CFG::nonterminal_set_type CFG::get_heads() const {
    nonterminal_set_type heads;
    for (auto const &head : get_head_view()) {
      heads.insert(head);
    }
    return heads;
  }
  CFG::terminal_set_type CFG::get_terminals() const {
    terminal_set_type terminals;
    for (auto const &[head, body] : productions_view()) {
      for (auto const s : body.get_terminal_view()) {
        terminals.insert(s);
      }
    }
    return terminals;
  }

  CFG::nonterminal_set_type CFG::get_nonterminals() const {
    nonterminal_set_type nonterminals;
    for (auto const &[head, bodies] : productions) {
      nonterminals.insert(head);
      for (auto const &body : bodies) {
        for (auto const s : body.get_nonterminal_view()) {
          nonterminals.insert(s);
        }
      }
    }
    return nonterminals;
  }

  bool CFG::has_production(const CFG_production &production) const {
    auto it = productions.find(production.get_head());
    return it != productions.end() &&
           std::ranges::find(it->second, production.get_body()) !=
               it->second.end();
  }

  void CFG::normalize_productions() {
    std::erase_if(productions,
                  [](const auto &bodyes) { return bodyes.second.empty(); });
    first_sets.clear();
  }

  void CFG::eliminate_useless_symbols() {
    if (productions.empty()) {
      return;
    }

    // eliminate unreachable heads from start symbol
    nonterminal_set_type reachable_heads{start_symbol};
    while (true) {
      auto prev_size = reachable_heads.size();
      for (const auto &[head, bodies] : productions) {
        if (!reachable_heads.contains(head)) {
          continue;
        }
        for (const auto &body : bodies) {
          for (auto const &nt : body.get_nonterminal_view()) {
            reachable_heads.insert(nt);
          }
        }
      }
      if (reachable_heads.size() == prev_size) {
        break;
      }
    }
    std::erase_if(productions, [&reachable_heads](const auto &production) {
      return !reachable_heads.contains(production.first);
    });
    // eliminate unused productions
    nonterminal_set_type in_use_heads;
    decltype(productions) new_productions;
    bool has_new_production = true;
    while (has_new_production) {
      has_new_production = false;
      for (auto &[head, bodies] : productions) {
        std::vector<decltype(bodies.begin())> iterator_to_remove;
        for (auto it = bodies.begin(); it != bodies.end(); ++it) {
          if (std::ranges::all_of(*it, [&in_use_heads](auto const &symbol) {
                return symbol.is_terminal() ||
                       in_use_heads.contains(*symbol.get_nonterminal_ptr());
              })) {
            in_use_heads.insert(head);
            new_productions[head].emplace(*it);
            has_new_production = true;
            iterator_to_remove.push_back(it);
          }
        }
        for (auto it : iterator_to_remove) {
          bodies.erase(it);
        }
      }
    }
    productions = std::move(new_productions);

    // eliminate empty production only head
    nonterminal_set_type non_empty_heads;
    auto flag = true;
    while (flag) {
      flag = false;
      for (auto &[head, bodies] : productions) {
        for (const auto &body : bodies) {
          if (std::ranges::any_of(body, [&non_empty_heads](auto const &symbol) {
                return symbol.is_terminal() ||
                       non_empty_heads.contains(*symbol.get_nonterminal_ptr());
              })) {
            if (non_empty_heads.emplace(head).second) {
              flag = true;
            }
          }
        }
      }
    }

    for (auto const &head : get_heads()) {
      if (head == start_symbol) {
        continue;
      }
      if (non_empty_heads.contains(head)) {
        continue;
      }
      productions.erase(head);
      for (auto &[_, bodies] : productions) {
        modify_body_set(bodies, [&head](auto &body) {
          std::erase(body, head);
          return true;
        });
      }
    }
    // eliminate productions A => A
    for (auto &[head, bodies] : productions) {
      auto const &real_head = head;
      std::erase_if(bodies, [&real_head](const auto body) {
        return body.size() == 1 && body[0] == real_head;
      });
    }
  }
  bool CFG::has_left_recursion() const {
    auto head_dependency = get_head_dependency();
    for (auto const &[head, derivations] : head_dependency) {
      if (derivations.contains(head)) {
        std::cerr << "left recursion head is " << head << std::endl;
        return true;
      }
    }
    return false;
  }

  void CFG::eliminate_left_recursion(std::vector<nonterminal_type> old_heads) {

    if (old_heads.empty()) {
      for (const auto &[head, _] : productions) {
        old_heads.push_back(head);
      }
    }

    auto const eliminate_immediate_left_recursion =
        [this](const nonterminal_type &head) {
          auto new_head = get_new_head(head);
          auto &bodies = productions[head];
          production_body_set_type new_bodies;

          modify_body_set(bodies, [&](auto &body) {
            if (!body.empty() && body.front() == head) {
              body.erase(body.begin());
              body.emplace_back(new_head);
              new_bodies.emplace(std::move(body));
              body = {};
            }
            return true;
          });

          if (new_bodies.empty()) {
            return;
          }
          new_bodies.emplace();
          productions[new_head] = std::move(new_bodies);

          modify_body_set(bodies, [&](auto &body) {
            body.emplace_back(new_head);
            return true;
          });
        };

    for (size_t i = 0; i < old_heads.size(); i++) {
      for (size_t j = 0; j < i; j++) {
        auto &bodies = productions[old_heads[i]];
        modify_body_set(bodies, [&](auto &body) {
          if (body.empty()) {
            return true;
          }

          if (body.front() != old_heads[j]) {
            return true;
          }
          // Ai -> Aj，替换
          for (const auto &head_j_body : productions[old_heads[j]]) {
            auto new_body = head_j_body;
            new_body.insert(new_body.end(), body.begin() + 1, body.end());
            bodies.emplace(std::move(new_body));
          }
          return false;
        });
      }
      eliminate_immediate_left_recursion(old_heads[i]);
    }
    eliminate_useless_symbols();
    normalize_productions();
  }

  void CFG::left_factoring() {

    normalize_productions();
    const auto left_factoring_nonterminal =
        [this](const nonterminal_type &head) -> nonterminal_type {
      auto &old_bodies = productions[head];
      if (old_bodies.size() < 2) {
        return {};
      }
      std::vector<CFG_production::body_type> bodies;
      std::ranges::move(old_bodies, std::back_inserter(bodies));
      old_bodies.clear();
      auto common_prefix = bodies.front();
      std::vector<size_t> indexes{0};
      for (size_t j = 1; j < bodies.size(); j++) {
        size_t i = 0;
        for (; i < common_prefix.size() && i < bodies[j].size(); i++) {
          if (common_prefix[i] != bodies[j][i]) {
            break;
          }
        }
        if (i > 0) {
          common_prefix.resize(i);
          indexes.push_back(j);
          continue;
        }

        if (!common_prefix.empty() && indexes.size() > 1) {
          break;
        }
        common_prefix = bodies[j];
        indexes = {j};
      }
      if (!common_prefix.empty() && indexes.size() > 1) {
        auto new_head = get_new_head(head);
        for (auto &index : indexes) {
          auto &body = bodies[index];

          productions[new_head].emplace(
              std::move_iterator(body.begin() + static_cast<std::ptrdiff_t>(
                                                    common_prefix.size())),
              std::move_iterator(body.end()));
          body.erase(body.begin() +
                         static_cast<std::ptrdiff_t>(common_prefix.size()),
                     body.end());
          body.emplace_back(new_head);
        }
        std::ranges::move(bodies,
                          std::inserter(old_bodies, old_bodies.begin()));
        return new_head;
      }
      std::ranges::move(bodies, std::inserter(old_bodies, old_bodies.begin()));
      return {};
    };

    for (auto head : get_heads()) {
      while (true) {
        head = left_factoring_nonterminal(head);
        if (head.empty()) {
          break;
        }
      }
    }

    normalize_productions();
  }

  const std::unordered_map<CFG::nonterminal_type,
                           std::pair<CFG::terminal_set_type, bool>> &
  CFG::first() const {

    if (!first_sets.empty()) {
      return first_sets;
    }

    // process all terminals
    for (auto const &[head, bodies] : productions) {
      for (auto const &body : bodies) {
        auto &[first_set, epsilon_in_first] = first_sets[head];
        if (body.empty()) {
          epsilon_in_first = true;
        } else if (body[0].is_terminal()) {
          first_set.insert(body[0].get_terminal());
        }
      }
    }

    bool add_new_terminal = true;
    while (add_new_terminal) {

      add_new_terminal = false;

      for (auto const &[head, bodies] : productions) {
        auto &[first_set, epsilon_in_first] = first_sets[head];
        for (auto const &body : bodies) {
          size_t i = 0;
          for (; i < body.size(); i++) {
            if (body[i].is_terminal()) {
              if (first_set.insert(body[i].get_terminal()).second) {
                add_new_terminal = true;
              }
              break;
            }

            auto const &nonterminal = body[i].get_nonterminal();

            for (auto const &first_terminal : first_sets[nonterminal].first) {
              if (first_set.insert(first_terminal).second) {
                add_new_terminal = true;
              }
            }
            if (!first_sets[nonterminal].second) {
              break;
            }
          }
          if (i == body.size()) {
            if (!epsilon_in_first) {
              epsilon_in_first = true;
              add_new_terminal = true;
            }
          }
        }
      }
    }

    return first_sets;
  }

  std::pair<CFG::terminal_set_type, bool>
  CFG::first(const grammar_symbol_const_span_type &alpha) const {
    if (alpha.empty()) {
      return {{}, true};
    }

    first();
    terminal_set_type view_first_set;
    for (auto const &symbol : alpha) {
      if (symbol.is_terminal()) {
        view_first_set.insert(symbol.get_terminal());
        return {view_first_set, false};
      }
      const auto &nonterminal = *symbol.get_nonterminal_ptr();

      auto it = first_sets.find(nonterminal);
      assert(it != first_sets.end());
      auto [first_set, epsilon_in_first] = it->second;
      view_first_set.merge(first_set);
      if (!epsilon_in_first) {
        return {view_first_set, false};
      }
    }
    return {view_first_set, true};
  }

  std::unordered_map<CFG::nonterminal_type, CFG::terminal_set_type>
  CFG::follow() const {
    std::unordered_map<nonterminal_type, terminal_set_type> follow_sets;

    follow_sets[start_symbol].insert(ALPHABET::endmarker);

    bool has_add = false;
    do {
      has_add = false;
      for (const auto &[head, bodies] : productions) {
        for (const auto &body : bodies) {
          for (size_t i = 0; i < body.size(); i++) {
            if (body[i].is_terminal()) {
              continue;
            }

            auto const &[first_set, epsilon_in_first] =
                first(grammar_symbol_const_span_type(body).subspan(i + 1));

            const auto &nonterminal = *(body[i].get_nonterminal_ptr());
            auto &follow_set = follow_sets[nonterminal];
            for (auto const &terminal : first_set) {
              if (follow_set.insert(terminal).second) {
                has_add = true;
              }
            }

            if (epsilon_in_first) {
              for (auto const &terminal : follow_sets[head]) {
                if (follow_set.insert(terminal).second) {
                  has_add = true;
                }
              }
            }
          }
        }
      }
    } while (has_add);

    return follow_sets;
  }

  std::unordered_map<CFG::nonterminal_type, CFG::nonterminal_set_type>
  CFG::get_head_dependency() const {
    std::unordered_map<CFG::nonterminal_type, CFG::nonterminal_set_type> result;

    for (const auto &[head, bodies] : productions) {
      for (const auto &body : bodies) {
        if (!body.empty() && body[0].is_nonterminal()) {
          result[head].insert(body[0].get_nonterminal());
        }
      }
    }

    bool flag = true;
    while (flag) {
      flag = false;
      for (auto &[k, v_set] : result) {
        for (auto &v : v_set) {
          if (v == k) {
            continue;
          }
          auto it = result.find(v);
          if (it == result.end()) {
            continue;
          }
          auto prev_size = v_set.size();
          v_set.merge(CFG::nonterminal_set_type(it->second));
          if (v_set.size() > prev_size) {
            flag = true;
          }
        }
      }
    }
    return result;
  }
  void CFG::normalize_start_symbol() {
    if (!old_start_symbol.empty()) {
      return;
    }
    old_start_symbol = start_symbol;
    start_symbol = get_new_head(start_symbol);
    productions[start_symbol].emplace(
        CFG_production::body_type{old_start_symbol});
    return;
  }
  void CFG::remove_head(const nonterminal_type &head) {
    productions.erase(head);
    if (head == start_symbol) {
      if (old_start_symbol.empty()) {
        throw exception::no_CFG("no productions for start symbol");
      }
      start_symbol = old_start_symbol;
      old_start_symbol.clear();
    }
  }
  bool CFG::contains(const grammar_symbol_type &grammar_symbol) const {
    for (auto const &[head, bodies] : productions) {
      if (head == grammar_symbol) {
        return true;
      }
      for (auto const &body : bodies) {
        if (std::ranges::find(body, grammar_symbol) != body.end()) {
          return true;
        }
      }
    }
    return false;
  }
  std::string CFG::MMA_draw() const {
    // by convention,we print start symbol first.
    std::string cmd = "TableForm[{";
    for (size_t i = 0; i < 2; i++) {
      for (auto const &[head, bodies] : productions) {
        if (i == 0 && head != start_symbol) {
          continue;
        }
        if (i == 1 && head == start_symbol) {
          continue;
        }
        for (auto const &body : bodies) {
          cmd += CFG_production(head, body).MMA_draw(*alphabet, i == 0);
          cmd.push_back(',');
        }
      }
    }
    cmd.back() = '}';
    cmd.push_back(']');
    return cmd;
  }
  const CFG::production_body_set_type &
  CFG::get_bodies(const nonterminal_type &head) const {
    auto it = productions.find(head);
    if (it == productions.end()) {
      throw std::runtime_error("no bodies");
    }
    return it->second;
  }

  ALPHABET_ptr CFG::get_terminal_alphabet() const {
    auto terminal_set = get_terminals();
    if (terminal_set.size() < alphabet->size()) {
      return std::make_shared<sub_alphabet>(
          alphabet,
          std::set<terminal_type>(terminal_set.begin(), terminal_set.end()));
    }
    return alphabet;
  }

  std::shared_ptr<map_alphabet<std::string>>
  CFG::get_nonterminal_alphabet() const {
    auto max_symbol = get_alphabet().get_max_symbol();
    std::unordered_map<symbol_type, nonterminal_type> symbol_to_nonterminal;
    auto nonterminals = get_nonterminals();
    for (auto &nonterminal : nonterminals) {
      max_symbol++;
      symbol_to_nonterminal.emplace(max_symbol, nonterminal);
    }

    auto nonterminal_alphabet_ptr = std::make_shared<map_alphabet<std::string>>(
        symbol_to_nonterminal, "alphabet_of_nonterminals");
    nonterminal_alphabet_ptr->set_MMA_draw_fun(
        [](auto const &nonterminal_alphabet, auto symbol) {
          return grammar_symbol_type(
                     reinterpret_cast<const map_alphabet<std::string> *>(
                         &nonterminal_alphabet)
                         ->get_data(symbol))
              .MMA_draw(nonterminal_alphabet);
        });
    return nonterminal_alphabet_ptr;
  }
  ALPHABET_ptr CFG::get_full_alphabet() const {
    return std::make_shared<union_alphabet>(get_terminal_alphabet(),
                                            get_nonterminal_alphabet());
  }
  void CFG::modify_body_set(
      production_body_set_type &body_set,
      const std::function<bool(CFG_production::body_type &)> &fun) {
    std::vector<CFG_production::body_type> body_vector{
        std::move_iterator{body_set.begin()},
        std::move_iterator{body_set.end()}};
    body_set.clear();
    for (auto &body : body_vector) {
      if (fun(body)) {
        body_set.emplace(std::move(body));
      }
    }
  }
} // namespace cyy::computation
