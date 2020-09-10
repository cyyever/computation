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
  std::string CFG::parse_node::MMA_draw(const ALPHABET &alphabet_) const {
    auto [vertex_cmd, edge_cmd, _] = MMA_draw_edge(alphabet_, 0);
    std::string cmd =
        "TreeGraph[{" + edge_cmd + "},VertexLabels ->{" + vertex_cmd +
        "},VertexStyle -> {0->Orange},EdgeStyle->Thin,ImageSize->Large]";
    return cmd;
  }
  std::tuple<std::string, std::string, size_t>
  CFG::parse_node::MMA_draw_edge(const ALPHABET &alphabet_,
                                 size_t vertex_id) const {
    auto vertex_cmd = std::to_string(vertex_id) + "->" +
                      grammar_symbol_type(grammar_symbol).MMA_draw(alphabet_);
    if (children.empty()) {
      if (grammar_symbol.is_terminal()) {
        return {vertex_cmd, "", vertex_id};
      }
      vertex_cmd += "," + std::to_string(vertex_id + 1) +
                    "-> Style[\\[Epsilon], Bold, Italic]";
      return {vertex_cmd,
              std::to_string(vertex_id) + "->" + std::to_string(vertex_id + 1),
              vertex_id + 1};
    }
    std::string edge_cmd;
    auto last_vertex_id = vertex_id;
    for (size_t i = 0; i < children.size(); i++) {
      if (i != 0) {
        edge_cmd.push_back(',');
      }
      edge_cmd +=
          std::to_string(vertex_id) + "->" + std::to_string(last_vertex_id + 1);
      std::string sub_vertex_cmd, sub_edge_cmd;
      std::tie(sub_vertex_cmd, sub_edge_cmd, last_vertex_id) =
          children[i]->MMA_draw_edge(alphabet_, last_vertex_id + 1);
      vertex_cmd += "," + sub_vertex_cmd;
      if (!sub_edge_cmd.empty()) {
        edge_cmd += "," + sub_edge_cmd;
      }
    }
    return {vertex_cmd, edge_cmd, last_vertex_id};
  }

} // namespace cyy::computation
