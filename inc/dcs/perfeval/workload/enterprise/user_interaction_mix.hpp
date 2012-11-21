/**
 * \file dcs/pefeval/workload/enterprise/user_interaction_mix_model.hpp
 *
 * \brief Model for traffic mixes.
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

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_INTERACTION_MIX_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_INTERACTION_MIX_HPP


#include <cstddef>
#include <dcs/math/stats/distribution/discrete.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <utility>
#include <vector>


namespace dcs { namespace perfeval { namespace workload { namespace enterprise {

template <
	//typename RequestCategoryT,
	typename IntT,
	typename RealT
>
class user_interaction_mix
{
	public: typedef IntT int_type;
	public: typedef RealT real_type;


	public: template <typename ForwardIterator1T, typename ForwardIterator2T>
		user_interaction_mix(ForwardIterator1T first_req_cat, ForwardIterator1T last_req_cat, ForwardIterator2T first_weight, ForwardIterator2T last_weight)
		: req_categories_(first_req_cat, last_req_cat),
		  requests_dist_(first_weight, last_weight)
	{
		// empty
	}


	public: template <typename UniformRandomGeneratorT>
		int_type generate(UniformRandomGeneratorT& rng) const
	{
		::std::size_t req_idx;

		req_idx = ::dcs::math::stats::rand(requests_dist_, rng);

		return req_categories_[req_idx];
	}


	private: ::std::vector<int_type> req_categories_;
	private: ::dcs::math::stats::discrete_distribution<real_type> requests_dist_;
};

}}}} // Namespace dcs::perfeval::workload::enterprise


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_INTERACTION_MIX_HPP
