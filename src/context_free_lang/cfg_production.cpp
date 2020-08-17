/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "cfg_production.hpp"

namespace cyy::computation {

  bool CFG_production::is_epsilon() const { return body.empty(); }

  void CFG_production::print(std::ostream &os, const ALPHABET &alphabet) const {
    os << head << " -> ";
    print_body(os, alphabet);
    os << '\n';
  }
  void CFG_production::print_body(std::ostream &os,
                                  const ALPHABET &alphabet) const {
    if (body.empty()) {
      os << "'epsilon'";
    } else {
      for (const auto &grammal_symbol : body) {
        grammal_symbol.print(os, alphabet);
        os << ' ';
      }
    }
  }
  std::string CFG_production::MMA_draw(
      const ALPHABET &alphabet, bool emphasize_head,
      std::function<std::string(size_t)> pos_callback) const {
    std::string cmd = "Rule[";
    if (emphasize_head) {
      cmd += "Style[";
    }
    cmd += grammar_symbol_type(head).MMA_draw(alphabet);
    if (emphasize_head) {
      cmd += ",Red]";
    }
    cmd += ",{";
    for (size_t i = 0; i <= body.size(); i++) {
      auto sub_cmd = pos_callback(i);
      if (!sub_cmd.empty()) {
        cmd += sub_cmd;
        cmd.push_back(',');
      }
      if (i < body.size()) {
        cmd += body[i].MMA_draw(alphabet);
        cmd.push_back(',');
      }
    }
    if (cmd.back() == ',') {
      cmd.pop_back();
    }
    cmd += "}]";
    return cmd;
  }
} // namespace cyy::computation
