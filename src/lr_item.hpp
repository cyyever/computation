/*!
 * \file lr_item.hpp
 *
 * \author cyy
 * \date 2018-04-21
 */

#pragma once

#include <functional>
#include <set>
#include <unordered_set>

#include "cfg.hpp"

namespace cyy::lang {

struct LR_0_item {
  CFG::production_type production;
  size_t dot_pos;
  bool operator==(const LR_0_item &rhs) const {
    return std::tie(production,dot_pos) == std::tie(rhs.production,rhs.dot_pos);
  }
};

} // namespace cyy::lang

namespace std {
template <> struct hash<cyy::lang::LR_0_item> {
  size_t operator()(const cyy::lang::LR_0_item &x) const {
    auto hash_value =
        ::std::hash<decltype(x.production.first)>()(x.production.first) ^

        ::std::hash<decltype(x.production.second.size())>()(
            x.production.second.size()) ^
        ::std::hash<decltype(x.dot_pos)>()(x.dot_pos);
    return hash_value;
  }
};
} // namespace std

namespace cyy::lang {

struct LR_0_item_set {

 void  add_kernel_item(const       
  CFG &cfg,
  LR_0_item kernel_item
  ) {
   std::set<CFG::nonterminal_type> tmp_nonkernel_items;
   if (kernel_item.dot_pos < kernel_item.production.second.size()) {
     auto const &symbol = kernel_item.production.second[kernel_item.dot_pos];
     if (auto ptr=std::get_if<CFG::nonterminal_type>(&symbol);ptr) {
       tmp_nonkernel_items.insert(*ptr);
     }
   }

   bool has_added=true;
   while (has_added) {
     has_added = false;

     for (const auto &nonkernel_item : tmp_nonkernel_items) {
       auto it = cfg.get_productions().find(nonkernel_item);

       for (auto const &body : it->second) {
	 if(cfg.is_epsilon(body[0])) {
	    kernel_items.emplace(LR_0_item{ {it->first,body}   ,1});
	  continue;
	 }

	 if (auto ptr=std::get_if<CFG::nonterminal_type>(&body[0]);ptr) {
	   if (tmp_nonkernel_items.insert(*ptr)
	       .second) {
	     has_added = true;
	   }
	 }
       }
     }
   }
   kernel_items.emplace(std::move(   kernel_item));
   nonkernel_items.merge(tmp_nonkernel_items);
}

  std::unordered_set<LR_0_item> kernel_items;
  std::set<CFG::nonterminal_type> nonkernel_items;

  bool operator==(const LR_0_item_set &rhs) const {
    return kernel_items == rhs.kernel_items;
  }
  bool empty() const { return kernel_items.empty(); }

};
} // namespace cyy::lang

namespace std {
template <> struct hash<cyy::lang::LR_0_item_set> {
  size_t operator()(const cyy::lang::LR_0_item_set &x) const {
    auto hash_value =
        ::std::hash<decltype(x.kernel_items.size())>()(x.kernel_items.size());
    return hash_value;
  }
};
} // namespace std

namespace cyy::lang {
struct LR_1_item {
  LR_0_item item;
  CFG::terminal_type lookahead;
  bool operator==(const LR_1_item &rhs) const {
    return std::tie(item,lookahead) == std::tie(rhs.item,rhs.lookahead);
  }
};

} // namespace cyy::lang

namespace std {
template <> struct hash<cyy::lang::LR_1_item> {
  size_t operator()(const cyy::lang::LR_1_item &x) const {
    auto hash_value =
        ::std::hash<decltype(x.item)>()(x.item) ^
        ::std::hash<decltype(x.lookahead)>()(x.lookahead);
    return hash_value;
  }
};
} // namespace std

namespace cyy::lang {
  using LR_1_item_set=std::unordered_set<LR_1_item>;
}


namespace std {
template <> struct hash<cyy::lang::LR_1_item_set> {
  size_t operator()(const cyy::lang::LR_1_item_set &x) const {
    auto hash_value =
      ::std::hash<decltype(x.size())>()(x.size());
    return hash_value;
  }
};
} // namespace std
