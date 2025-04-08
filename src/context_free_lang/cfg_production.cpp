/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "cfg_production.hpp"

#include <cyy/algorithm/hash.hpp>

namespace cyy::computation {

  bool CFG_production::is_epsilon() const noexcept { return body.empty(); }

  std::string CFG_production::to_string(const ALPHABET &alphabet) const {
    return std::format("{} -> {}\n", head, body_to_string(alphabet));
  }

  std::string CFG_production::body_to_string(const ALPHABET &alphabet) const {
    if (body.empty()) {
      return "'epsilon'";
    }
    std::string str;
    for (const auto &grammal_symbol : body) {
      str += grammal_symbol.to_string(alphabet);
      str.push_back(' ');
    }
    return str;
  }
  std::string CFG_production::MMA_draw(
      const ALPHABET &alphabet, bool emphasize_head,
      const std::function<std::string(size_t)> &pos_callback) const {
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
namespace std {
  template <> struct hash<cyy::computation::CFG_production> {
    std::size_t operator()(const cyy::computation::CFG_production &x) const {
      size_t seed = 0;
      auto const &head = x.get_head();
      auto const &body = x.get_body();
      boost::hash_combine(
          seed, std::hash<cyy::computation::CFG_production::head_type>()(head));
      boost::hash_combine(
          seed, std::hash<cyy::computation::CFG_production::body_type>()(body));
      return seed;
    }
  };
} // namespace std
