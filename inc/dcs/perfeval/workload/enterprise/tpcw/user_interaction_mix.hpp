/**
 * \file dcs/perfeval/workload/enterprise/tpcw/user_interaction_mix.hpp
 *
 * \brief User interaction mixes related to the TPC-W benchmark.
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

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_TPCW_USER_INTERACTION_MIX_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_TPCW_USER_INTERACTION_MIX_HPP


#include <dcs/perfeval/workload/enterprise/user_interaction_mix.hpp>
#include <dcs/perfeval/workload/enterprise/tpcw/request_category.hpp>
#include <vector>


namespace dcs { namespace perfeval { namespace workload { namespace enterprise {

template <typename IntT, typename RealT>
user_interaction_mix<IntT,RealT> tpcw_browsing_mix()
{
	static ::std::vector<RealT> weights;
	if (!weights.size())
	{
		weights.reserve(14);
		weights.push_back(0.29);
		weights.push_back(0.11);
		weights.push_back(0.11);
		weights.push_back(0.21);
		weights.push_back(0.12);
		weights.push_back(0.11);
		weights.push_back(0.02);
		weights.push_back(0.0082);
		weights.push_back(0.0075);
		weights.push_back(0.0069);
		weights.push_back(0.0030);
		weights.push_back(0.0025);
		weights.push_back(0.0010);
		weights.push_back(0.0009);
	}

	return user_interaction_mix<IntT,RealT>(
		tpcw_request_categories().begin(),
		tpcw_request_categories().end(),
		weights.begin(),
		weights.end()
	);
}


template <typename IntT, typename RealT>
user_interaction_mix<IntT,RealT> tpcw_shopping_mix()
{
	static ::std::vector<RealT> weights;
	if (!weights.size())
	{
		weights.reserve(14);
		weights.push_back(0.16);
		weights.push_back(0.05);
		weights.push_back(0.05);
		weights.push_back(0.17);
		weights.push_back(0.2);
		weights.push_back(0.17);
		weights.push_back(0.116);
		weights.push_back(0.03);
		weights.push_back(0.026);
		weights.push_back(0.012);
		weights.push_back(0.0075);
		weights.push_back(0.0066);
		weights.push_back(0.001);
		weights.push_back(0.0009);
	}

	return user_interaction_mix<IntT,RealT>(
		tpcw_request_categories().begin(),
		tpcw_request_categories().end(),
		weights.begin(),
		weights.end()
	);
}


template <typename IntT, typename RealT>
user_interaction_mix<IntT,RealT> tpcw_ordering_mix()
{
	static ::std::vector<RealT> weights;
	if (!weights.size())
	{
		weights.reserve(14);
		weights.push_back(0.0912);
		weights.push_back(0.0046);
		weights.push_back(0.0046);
		weights.push_back(0.1235);
		weights.push_back(0.1453);
		weights.push_back(0.1308);
		weights.push_back(0.1353);
		weights.push_back(0.1286);
		weights.push_back(0.1273);
		weights.push_back(0.1018);
		weights.push_back(0.0025);
		weights.push_back(0.0022);
		weights.push_back(0.0012);
		weights.push_back(0.0011);
	}

	return user_interaction_mix<IntT,RealT>(
		tpcw_request_categories().begin(),
		tpcw_request_categories().end(),
		weights.begin(),
		weights.end()
	);
}

}}}} // Namespace dcs::perfeval::workload::enterprise


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_TPCW_WORKLOAD_HPP
