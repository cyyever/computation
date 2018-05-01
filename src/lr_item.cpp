/*!
 * \file lr_item.cpp
 *
 * \author cyy
 * \date 2018-04-21
 */


#include "lr_item.hpp"

namespace cyy::lang {

void LR_0_item_set ::add_kernel_item(const       
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

} // namespace cyy::lang
