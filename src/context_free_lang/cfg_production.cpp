/*!
 * \file grammar.hpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include "cfg_production.hpp"
#ifdef __cpp_lib_format
#include <format>
#define fmt std
#else
#include <fmt/format.h>
#endif

namespace cyy::computation {

  bool CFG_production::is_epsilon() const noexcept { return body.empty(); }

  std::string CFG_production::to_string(const ALPHABET &alphabet) const {
    return fmt::format("{} -> {}\n", head, body_to_string(alphabet));
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
