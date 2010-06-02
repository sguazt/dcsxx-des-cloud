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
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
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
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename RealT=double>
class any_cost_model
{
	public: typedef RealT real_type;


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


	public: real_type cost(::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<real_type> > const& measures) const
	{
		return ptr_model_->cost(measures);
	}


	public: bool satisfied(::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<real_type> > const& measures) const
	{
		return ptr_model_->satisfied(measures);
	}


//	public: real_type cost(::dcs::iterator::iterator_range< ::dcs::iterator::any_forward_iterator<real_type const> > const& measures) const
//	{
//		return ptr_model_->cost(measures);
//	}


	// convenience method
	public: template <typename ForwardIteratorT>
		real_type cost(::dcs::iterator::iterator_range<ForwardIteratorT> const& measures) const
	{
		return ptr_model_->cost(
			::dcs::iterator::make_iterator_range(
				::dcs::iterator::make_any_forward_iterator<ForwardIteratorT>(measures.begin()),
				::dcs::iterator::make_any_forward_iterator<ForwardIteratorT>(measures.end())
			)
		);
	}


	// convenience method
	public: template <typename ForwardIteratorT>
		bool satisfied(::dcs::iterator::iterator_range<ForwardIteratorT> const& measures) const
	{
		return ptr_model_->satisfied(
			::dcs::iterator::make_iterator_range(
				::dcs::iterator::make_any_forward_iterator<ForwardIteratorT>(measures.begin()),
				::dcs::iterator::make_any_forward_iterator<ForwardIteratorT>(measures.end())
			)
		);
	}


	private: ::dcs::shared_ptr< base_cost_model<real_type> > ptr_model_;
};


template <typename SlaCostModelT, typename SlaCostModelTraitsT=typename ::dcs::type_traits::remove_reference<SlaCostModelT>::type>
struct make_any_cost_model_type
{
	typedef any_cost_model<typename SlaCostModelTraitsT::real_type> type;
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

} // Namespace detail


template <typename SlaCostModelT>
typename make_any_cost_model_type<SlaCostModelT>::type make_any_cost_model(SlaCostModelT model)
{
	return detail::make_any_cost_model_impl<SlaCostModelT>::apply(model);
}

}}} // Namespace dcs::perfeval::sla


#endif // DCS_PERFEVAL_SLA_ANY_COST_MODEL_HPP
