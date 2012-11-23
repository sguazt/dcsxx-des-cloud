/**
 * \file dcs/perfeval/workload/enterprise/user_request.hpp
 *
 * \brief A user requests.
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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_REQUEST_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_REQUEST_HPP


namespace dcs { namespace perfeval { namespace workload { namespace enterprise {

//template <typename RequestCategoryT, typename RealT>
template <typename IntT, typename RealT>
class user_request
{
	public: typedef IntT int_type;
	public: typedef RealT real_type;


	public: user_request()
	{
		// Empty
	}


	public: user_request(int_type const& category, real_type iatime)
		: category_(category),
		  iatime_(iatime)
	{
		// empty
	}


	public: int_type category() const
	{
		return category_;
	}


	public: real_type interarrival_time() const
	{
		return iatime_;
	}


	private: int_type category_;
	private: real_type iatime_;
};

}}}} // Namespace dcs::perfeval::workload::enterprise


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_USER_REQUEST_HPP
