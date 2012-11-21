/**
 * \file dcs/perfeval/workload/tpcw/enterprise/request_category.hpp
 *
 * \brief Categories defined by the TPC-W benchmark.
 *
 * Copyright (C) 2009-2010  Distributed Computing System (DCS) Group, Computer
 * Science Department - University of Piemonte Orientale, Alessandria (Italy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_TPCW_REQUEST_CATEGORY_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_TPCW_REQUEST_CATEGORY_HPP


#include <vector>


namespace dcs { namespace perfeval { namespace workload { namespace enterprise {

enum tpcw_request_category
{
	tpcw_home_request_category,
	tpcw_new_products_request_category,
	tpcw_best_sellers_request_category,
	tpcw_product_detail_request_category,
	tpcw_search_request_request_category,
	tpcw_search_results_request_category,
	tpcw_shopping_cart_request_category,
	tpcw_customer_registration_request_category,
	tpcw_buy_request_request_category,
	tpcw_buy_confirm_request_category,
	tpcw_order_inquiry_request_category,
	tpcw_order_display_request_category,
	tpcw_admin_request_request_category,
	tpcw_admin_confirm_request_category
};


::std::vector<tpcw_request_category> const& tpcw_request_categories()
{
	static ::std::vector<tpcw_request_category> categories;

	if (!categories.size())
	{
		categories.reserve(14);
		categories.push_back(tpcw_home_request_category);
		categories.push_back(tpcw_new_products_request_category);
		categories.push_back(tpcw_best_sellers_request_category);
		categories.push_back(tpcw_product_detail_request_category);
		categories.push_back(tpcw_search_request_request_category);
		categories.push_back(tpcw_search_results_request_category);
		categories.push_back(tpcw_shopping_cart_request_category);
		categories.push_back(tpcw_customer_registration_request_category);
		categories.push_back(tpcw_buy_request_request_category);
		categories.push_back(tpcw_buy_confirm_request_category);
		categories.push_back(tpcw_order_inquiry_request_category);
		categories.push_back(tpcw_order_display_request_category);
		categories.push_back(tpcw_admin_request_request_category);
		categories.push_back(tpcw_admin_confirm_request_category);
	}

	return categories;
}

}}}} // Namespace dcs::perfeval::workload::enterprise


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_TPCW_REQUEST_CATEGORY_HPP
