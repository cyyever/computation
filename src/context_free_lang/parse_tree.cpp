/*!
 * \file grammar.cpp
 *
 * \author cyy
 * \date 2018-03-04
 */

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <utility>

#include "cfg.hpp"

namespace cyy::computation {

  CFG::parse_node_ptr
  CFG::parse_node::make_parse_node(CFG::nonterminal_type head,
                                   CFG_production::body_span_type body) {

    auto node = std::make_shared<parse_node>(std::move(head));
    node->children.reserve(body.size());
    for (auto const &grammar_symbol : body) {
      node->children.push_back(std::make_shared<parse_node>(grammar_symbol));
    }
    return node;
  }
  std::string CFG::parse_node::MMA_draw(const ALPHABET &alphabet) const {
    std::string cmd = "TreeGraph[{" + MMA_draw_edge(alphabet, 0).first + "}]";
    return cmd;
  }
  std::pair<std::string, size_t>
  CFG::parse_node::MMA_draw_edge(const ALPHABET &alphabet,
                                 size_t vertex_id) const {
    if (children.empty()) {
      return {"", vertex_id};
    }
    std::stringstream is;
    std::string sub_cmd;
    auto last_vertex_id = vertex_id;
    for (size_t i = 0; i < children.size(); i++) {
      auto const &child = children[i];
      is << "Labeled[" << vertex_id << "->" << last_vertex_id + 1 << ','
         << child->grammar_symbol.MMA_draw(alphabet) << "],";
      std::tie(sub_cmd, last_vertex_id) =
          child->MMA_draw_edge(alphabet, last_vertex_id + 1);
      if (!sub_cmd.empty()) {
        is << sub_cmd << ',';
      }
    }
    auto cmd = is.str();
    cmd.pop_back();
    return {cmd, last_vertex_id};
  }

} // namespace cyy::computation
