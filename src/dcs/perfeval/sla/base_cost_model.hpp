/*
 * \file dcs/perfeval/sla/base_cost_model.hpp
 *
 * \brief Base class for SLA cost models implementing the SlaCostModel concept.
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

#ifndef DCS_PERFEVAL_SLA_BASE_COST_MODEL_HPP
#define DCS_PERFEVAL_SLA_BASE_COST_MODEL_HPP


#include <dcs/iterator/any_forward_iterator.hpp>
#include <dcs/iterator/iterator_range.hpp>


namespace dcs { namespace perfeval { namespace sla {

/**
 * \brief Base class for SLA cost models implementing the SlaCostModel concept.
 *
 * \tparam RealT The type for real numbers.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename RealT>
struct base_cost_model
{
	//[TODO]
	//DCS_CONCEPT_ASSERT((SlaCostModel<base_cost_model<RealT>))
	//[/TODO]

	/// Alias for the type of real numbers.
	typedef RealT real_type;

	/// The virtual destructor to make inherited classes destructible.
	virtual ~base_cost_model() { }

	virtual real_type cost(::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<real_type> > const& measures) const = 0;

	virtual bool satisfied(::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<real_type> > const& measures) const = 0;
};

}}} // Namespace dcs::perfeval::sla


#endif // DCS_PERFEVAL_SLA_BASE_COST_MODEL_HPP
