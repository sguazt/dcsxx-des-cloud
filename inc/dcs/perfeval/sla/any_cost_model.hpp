/**
 * \file dcs/perfeval/sla/any_cost_model.hpp
 *
 * \brief Generic (type-erased) SLA cost model implementing the SlaCostModel
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

#ifndef DCS_PERFEVAL_SLA_ANY_COST_MODEL_HPP
#define DCS_PERFEVAL_SLA_ANY_COST_MODEL_HPP


#include <dcs/perfeval/sla/base_cost_model.hpp>
#include <dcs/perfeval/sla/cost_model_adaptor.hpp>
#include <dcs/iterator/any_forward_iterator.hpp>
#include <dcs/iterator/iterator_range.hpp>
#include <dcs/memory.hpp>
#include <dcs/type_traits/remove_reference.hpp>
#include <dcs/util/holder.hpp>
#include <iterator>


namespace dcs { namespace perfeval { namespace sla {

/**
 * \brief Generic SLA cost model.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename CategoryT, typename ValueT, typename RealT=ValueT>
class any_cost_model
{
	public: typedef CategoryT metric_category_type;
	public: typedef ValueT value_type;
	public: typedef RealT real_type;
	private: typedef base_cost_model<metric_category_type,value_type,real_type> cost_model_type;
	private: typedef ::dcs::shared_ptr<cost_model_type> cost_model_pointer;


	public: any_cost_model()
	{
		// Empty
	}


	public: template <typename SlaCostModelT>
		any_cost_model(SlaCostModelT const& model)
		: ptr_model_(new cost_model_adaptor<SlaCostModelT>(model))
	{
		// empty
	}


	public: template <typename SlaCostModelT>
		any_cost_model(::dcs::util::holder<SlaCostModelT> const& wrap_model)
		: ptr_model_(new cost_model_adaptor<SlaCostModelT>(wrap_model.get()))
	{
		// empty
	}


	public: template <typename SlaCostModelT>
		void cost_model(SlaCostModelT const& model)
	{
		ptr_model_ = new cost_model_adaptor<SlaCostModelT>(model);
	}


	public: template <typename CheckerT>
		void add_slo(metric_category_type category, value_type value, CheckerT const& checker)
	{
		ptr_model_->add_slo(category, value, checker);
	}


	public: bool has_slo(metric_category_type category) const
	{
		return ptr_model_->has_slo(category);
	}


	public: ::std::vector<metric_category_type> slo_categories() const
	{
		return ptr_model_->slo_categories();
	}


	public: value_type slo_value(metric_category_type category) const
	{
		return ptr_model_->slo_value(category);
	}


	public: template <typename CategoryFwdIterT, typename MeasureFwdIterT>
		real_type score(CategoryFwdIterT category_first, CategoryFwdIterT category_last, MeasureFwdIterT metric_first) const
	{
		return ptr_model_->score(category_first, category_last, metric_first);
	}


	public: template <typename CategoryFwdIterT, typename MeasureFwdIterT>
		bool satisfied(CategoryFwdIterT category_first, CategoryFwdIterT category_last, MeasureFwdIterT metric_first) const
	{
		return ptr_model_->satisfied(category_first, category_last, metric_first);
	}


//	public: real_type cost(::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<real_type const> > const& measures) const
//	{
//		return ptr_model_->cost(measures);
//	}


	private: cost_model_pointer ptr_model_;
};


template <typename SlaCostModelT, typename SlaCostModelTraitsT=typename ::dcs::type_traits::remove_reference<SlaCostModelT>::type>
struct make_any_cost_model_type
{
	typedef any_cost_model<typename SlaCostModelTraitsT::metric_category_type,
						   typename SlaCostModelTraitsT::value_type,
						   typename SlaCostModelTraitsT::real_type> type;
};


namespace detail {

template <typename SlaCostModelT, typename SlaCostModelTraitsT=typename ::dcs::type_traits::remove_reference<SlaCostModelT>::type>
struct make_any_cost_model_impl;


template <typename SlaCostModelT, typename SlaCostModelTraitsT>
struct make_any_cost_model_impl
{
	typedef typename make_any_cost_model_type<SlaCostModelT,SlaCostModelTraitsT>::type any_cost_model_type;

	static any_cost_model_type apply(SlaCostModelT& model)
	{
		return any_cost_model_type(model);
	}
};


template <typename SlaCostModelT, typename SlaCostModelTraitsT>
struct make_any_cost_model_impl<SlaCostModelT&,SlaCostModelTraitsT>
{
	typedef typename make_any_cost_model_type<SlaCostModelT,SlaCostModelTraitsT>::type any_cost_model_type;

	static any_cost_model_type apply(SlaCostModelT& model)
	{
		::dcs::util::holder<SlaCostModelT&> wrap_model(model);
		return any_cost_model_type(wrap_model);
	}
};


template <typename CategoryT, typename ValueT, typename RealT>
struct make_any_cost_model_impl< any_cost_model<CategoryT,ValueT,RealT>, any_cost_model<CategoryT,ValueT,RealT> >
{
	typedef any_cost_model<CategoryT,ValueT,RealT> any_cost_model_type;

	static any_cost_model_type apply(any_cost_model<CategoryT,ValueT,RealT>& model)
	{
		return model;
	}

	static any_cost_model_type const& apply(any_cost_model<CategoryT,ValueT,RealT> const& model)
	{
		return model;
	}
};

} // Namespace detail


template <typename SlaCostModelT>
typename make_any_cost_model_type<SlaCostModelT>::type make_any_cost_model(SlaCostModelT model)
{
	return detail::make_any_cost_model_impl<SlaCostModelT>::apply(model);
}

}}} // Namespace dcs::perfeval::sla


#endif // DCS_PERFEVAL_SLA_ANY_COST_MODEL_HPP
