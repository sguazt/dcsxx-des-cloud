/**
 * \file dcs/perfeval/workload/enterprise/generator_adaptor.hpp
 *
 * \brief Adaptor for workload generator not inheriting from
 *  \c dcs::perfeval::workload::base_generator.
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

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_GENERATOR_ADAPTOR_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_GENERATOR_ADAPTOR_HPP


#include <dcs/perfeval/workload/enterprise/base_generator.hpp>
#include <dcs/perfeval/workload/enterprise/user_request.hpp>
#include <dcs/math/random/any_generator.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/add_reference.hpp>
#include <dcs/type_traits/remove_reference.hpp>


namespace dcs { namespace perfeval { namespace workload { namespace enterprise {

template <
	typename WorkloadGeneratorT,
	typename WorkloadGeneratorTraitsT=typename ::dcs::type_traits::remove_reference<WorkloadGeneratorT>::type
>
class generator_adaptor: public base_generator<
										typename WorkloadGeneratorTraitsT::int_type,
										typename WorkloadGeneratorTraitsT::real_type
									>
{
	public: typedef WorkloadGeneratorT generator_type;
	public: typedef typename WorkloadGeneratorTraitsT::int_type int_type;
	public: typedef typename WorkloadGeneratorTraitsT::real_type real_type;
	public: typedef typename ::dcs::type_traits::add_reference<WorkloadGeneratorT>::type generator_reference;
	public: typedef typename ::dcs::type_traits::add_reference<typename ::dcs::type_traits::add_const<WorkloadGeneratorT>::type>::type generator_const_reference;

	public: generator_adaptor(generator_const_reference generator)
		: generator_(generator)
	{
		// empty
	}


	//public: ::std::pair<request_category_type,real_type> operator()(::dcs::math::random::any_generator<real_type>& rng) const
	public: user_request<int_type,real_type> operator()(::dcs::math::random::any_generator<real_type>& rng) const
	{
		return generator_(rng);
	}


	private: generator_type generator_;
};

}}}} // Namespace dcs::perfeval::workload::enterprise


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_GENERATOR_ADAPTOR_HPP
