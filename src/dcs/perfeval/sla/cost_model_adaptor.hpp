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
#include <dcs/type_traits/remove_reference.hpp>
#include <vector>


namespace dcs { namespace perfeval { namespace sla {

template <
	typename SlaCostModelT,
	 typename SlaCostModelTraitsT=typename ::dcs::type_traits::remove_reference<SlaCostModelT>::type
>
class cost_model_adaptor: public base_cost_model<typename SlaCostModelTraitsT::metric_category_type,
												 typename SlaCostModelTraitsT::value_type,
												 typename SlaCostModelTraitsT::real_type>
{
	public: typedef SlaCostModelT model_type;
	public: typedef typename SlaCostModelTraitsT::metric_category_type metric_category_type;
	public: typedef typename SlaCostModelTraitsT::value_type value_type;
	public: typedef typename SlaCostModelTraitsT::real_type real_type;
	private: typedef typename ::dcs::type_traits::add_reference<SlaCostModelT>::type model_reference;
	private: typedef typename ::dcs::type_traits::add_reference<typename ::dcs::type_traits::add_const<SlaCostModelT>::type>::type model_const_reference;
	private: typedef base_cost_model<metric_category_type,value_type,real_type> base_type;
	private: typedef typename base_type::metric_category_iterator metric_category_iterator;
	private: typedef typename base_type::metric_iterator metric_iterator;
	private: typedef typename base_type::slo_model_type slo_model_type;


	public: cost_model_adaptor()
	{
	}


	public: cost_model_adaptor(model_const_reference model)
		: base_type(),
		  model_(model)
	{
		// empty;
	}


    private: void do_add_slo(slo_model_type const& slo)
	{
		model_.add_slo(slo.category(), slo.reference_value(), slo.checker());
	}


	private: bool do_has_slo(metric_category_type category) const
	{
		return model_.has_slo(category);
	}


	private: ::std::vector<metric_category_type> do_slo_categories() const
	{
		return model_.slo_categories();
	}


    private: real_type do_score(metric_category_iterator category_first, metric_category_iterator category_last, metric_iterator metric_first) const
	{
		return model_.score(category_first, category_last, metric_first);
	}


    private: bool do_satisfied(metric_category_iterator category_first, metric_category_iterator category_last, metric_iterator metric_first) const
	{
		return model_.satisfied(category_first, category_last, metric_first);
	}


	private: model_type model_;
};

}}} // Namespace dcs::perfeval::sla


#endif // DCS_PERFEVAL_SLA_COST_MODEL_ADAPTOR_HPP
