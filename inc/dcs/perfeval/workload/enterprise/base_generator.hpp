/*
 * \file dcs/perfeval/workload/enterprise/base_generator.hpp
 *
 * \brief Base class for workload generators implementing the WorkloadGenerator
 *  concept.
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

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_BASE_GENERATOR_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_BASE_GENERATOR_HPP


#include <dcs/math/random/any_generator.hpp>
#include <dcs/perfeval/workload/enterprise/user_request.hpp>
#include <utility>


namespace dcs { namespace perfeval { namespace workload { namespace enterprise {

/**
 * \brief Base class for workload generators implementing the WorkloadGenerator
 *  concept.
 *
 * \tparam RequestCategoryT The type for user request categories.
 * \tparam RealT The type for real numbers.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
//template <typename RequestCategoryT, typename RealT>
template <typename IntT, typename RealT>
struct base_generator
{
	//[TODO]
	//DCS_CONCEPT_ASSERT((EnergyModel<base_generator<RealT>))
	//[/TODO]

	/// Alias for the type of user request categories.
	typedef IntT int_type;
	/// Alias for the type of real numbers.
	typedef RealT real_type;

	/// The virtual destructor to make inherited classes destructible.
	virtual ~base_generator() { }

	/**
	 * \brief Randomly generate a new pair of request category and request
	 *  interarrival time.
	 * \param rng A generic uniform random number generator
	 * \return A random pair of request category and request interarrival time.
	 */
	//virtual ::std::pair<request_category_type,real_type> operator()(::dcs::math::random::any_generator<real_type>& rng) const = 0;
	virtual user_request<int_type,real_type> operator()(::dcs::math::random::any_generator<real_type>& rng) const = 0;
};

}}}} // Namespace dcs::perfeval::workload::enterprise


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_BASE_GENERATOR_HPP
