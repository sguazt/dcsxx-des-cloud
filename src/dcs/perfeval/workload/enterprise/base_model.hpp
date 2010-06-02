/*
 * \file dcs/perfeval/workload/enterprise/base_model.hpp
 *
 * \brief Base class for enterprise workload models implementing the
 *  EnterpriseWorkloadModel concept.
 *
 * Copyright (C) 2009-2010  Distributed Computing System (DCS) Group, Computer
 * Science Department - University of Piemonte Orientale, Alessandria (Italy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_BASE_MODEL_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_BASE_MODEL_HPP


#include <dcs/math/random/any_generator.hpp>
#include <utility>


namespace dcs { namespace perfeval { namespace workload { namespace enterprise {

/**
 * \brief Base class for enterprise workload models implementing the
 *  EnterpriseWorkloadModel concept.
 *
 * \tparam RealT The type for real numbers.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <
	typename RequestCategoryT,
	typename RealT
>
class base_model
{
	//[TODO]
	//DCS_CONCEPT_ASSERT((EnterpriseWorkloadModel<base_model<RequestCategoryT,RealT>))
	//[/TODO]

	/// Alias for the type of request categories.
	public: typedef RequestCategoryT request_category_type;
	/// Alias for the type of real numbers.
	typedef RealT real_type;

	/// The virtual destructor to make inherited classes destructible.
	public: virtual ~base_model() { }


	public: template <typename UniformRandomGeneratorT>
		::std::pair<request_category_type,real_type> generate(UniformRandomGeneratorT& rng) const
	{
		return do_generate(::dcs::math::random::make_any_generator<UniformRandomGeneratorT&>(rng));
	}


	private: virtual ::std::pair<request_category_type,real_type>  do_generate(::dcs::math::random::any_generator<real_type>& rng) const = 0;
};

}}}} // Namespace dcs::perfeval::workload::enterprise


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_BASE_MODEL_HPP
