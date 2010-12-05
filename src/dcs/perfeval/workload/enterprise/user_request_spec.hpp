/**
 * \file dcs/perfeval/workload/enterprise/user_request_spec.hpp
 *
 * \brief Model for user requests specification.
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

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_REQUEST_SPEC_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_REQUEST_SPEC_HPP


#include <dcs/perfeval/workload/enterprise/user_request.hpp>


namespace dcs { namespace perfeval { namespace workload { namespace enterprise {

//template <typename RequestCategoryT, typename RandomNumberDistributionT>
template <typename RandomNumberDistributionT, typename IntT>
class user_request_spec
{
	public: typedef IntT int_type;
	public: typedef RandomNumberDistributionT random_distribution_type;
	public: typedef typename random_distribution_type::value_type real_type;
	public: typedef user_request<int_type,real_type> request_type;


	public: user_request_spec()
	{
		// Empty
	}


	public: user_request_spec(int_type const& category, random_distribution_type const& iatime_dist)
		: category_(category),
		  iatime_dist_(iatime_dist)
	{
		// empty
	}


	public: int_type category() const
	{
		return category_;
	}


//	public: random_distribution_type& iatime_distribution()
//	{
//		return iatime_dist_;
//	}


	public: random_distribution_type const& iatime_distribution() const
	{
		return iatime_dist_;
	}


//	public: void iatime_distribution(random_distribution_type const& value)
//	{
//		iatime_dist_ = value;
//	}


	public: template <typename UniformRandomGeneratorT>
		request_type generate(UniformRandomGeneratorT& rng) const
	{
		real_type iatime = ::dcs::math::stats::rand(
			iatime_dist_,
			rng
		);

		return request_type(category_, iatime);
	}


	private: int_type category_;
	private: random_distribution_type iatime_dist_;
};

}}}} // Namespace dcs::perfeval::workload::enterprise


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_REQUEST_SPEC_HPP
