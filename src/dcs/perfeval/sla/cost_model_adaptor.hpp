/**
 * \file dcs/perfeval/sla/cost_model_adaptor.hpp
 *
 * \brief Adaptor for SLA cost models.
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

#ifndef DCS_PERFEVAL_SLA_COST_MODEL_ADAPTOR_HPP
#define DCS_PERFEVAL_SLA_COST_MODEL_ADAPTOR_HPP


#include <dcs/perfeval/sla/base_cost_model.hpp>
#include <dcs/iterator/any_forward_iterator.hpp>
#include <dcs/iterator/iterator_range.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/add_reference.hpp>


namespace dcs { namespace perfeval { namespace sla {

template <
	typename SlaCostModelT,
	 typename SlaCostModelTraitsT=typename ::dcs::type_traits::remove_reference<SlaCostModelT>::type
>
class cost_model_adaptor: public base_cost_model<typename SlaCostModelTraitsT::real_type>
{
	public: typedef typename SlaCostModelTraitsT::real_type real_type;
	private: typedef SlaCostModelT model_type;
	private: typedef typename ::dcs::type_traits::add_reference<SlaCostModelT>::type model_reference;
	private: typedef typename ::dcs::type_traits::add_reference<typename ::dcs::type_traits::add_const<SlaCostModelT>::type>::type model_const_reference;


	public: cost_model_adaptor()
	{
	}


	public: cost_model_adaptor(model_const_reference model)
		: model_(model)
	{
		// empty;
	}


	public: real_type cost(::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<real_type> > const& measures) const
	{
		return model_.cost(measures);
	}


	public: bool satisfied(::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<real_type> > const& measures) const
	{
		return model_.satisfied(measures);
	}


	private: model_type model_;
};

}}} // Namespace dcs::perfeval::sla


#endif // DCS_PERFEVAL_SLA_COST_MODEL_ADAPTOR_HPP
