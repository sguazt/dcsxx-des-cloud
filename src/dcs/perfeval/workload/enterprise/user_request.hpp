/**
 * \file dcs/perfeval/workload/enterprise/user_request.hpp
 *
 * \brief Model for user requests.
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

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_REQUEST_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_REQUEST_HPP


namespace dcs { namespace perfeval { namespace workload { namespace enterprise {

template <typename RequestCategoryT, typename RandomNumberDistributionT>
class user_request
{
	public: typedef RequestCategoryT request_category_type;
	public: typedef RandomNumberDistributionT distribution_type;


	public: user_request()
	{
	}


	public: user_request(request_category_type const& category, distribution_type const& iatime_dist)
		: category_(category),
		  iatime_dist_(iatime_dist)
	{
		// empty
	}


	public: request_category_type category() const
	{
		return category_;
	}


//	public: distribution_type& iatime_distribution()
//	{
//		return iatime_dist_;
//	}


	public: distribution_type const& iatime_distribution() const
	{
		return iatime_dist_;
	}


//	public: void iatime_distribution(distribution_type const& value)
//	{
//		iatime_dist_ = value;
//	}


	private: request_category_type category_;
	private: distribution_type iatime_dist_;
};

}}}} // Namespace dcs::perfeval::workload::enterprise


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_REQUEST_HPP
