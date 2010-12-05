/**
 * \file dcs/perfeval/workload/any_model.hpp
 *
 * \brief Generic (type-erased) enterprise workload model implementing the
 *  EnterpriseWorkloadModel concept
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

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_ANY_MODEL_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_ANY_MODEL_HPP


#include <dcs/perfeval/workload/enterprise/base_model.hpp>
#include <dcs/perfeval/workload/enterprise/model_adaptor.hpp>
#include <dcs/perfeval/workload/enterprise/user_request.hpp>
#include <dcs/memory.hpp>
#include <dcs/util/holder.hpp>
#include <utility>


namespace dcs { namespace perfeval { namespace workload { namespace enterprise {

/**
 * \brief Generic enterprise workload model.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <
	//typename RequestCategoryT,
	typename IntT,
	typename RealT
>
class any_model
{
	//public: typedef RequestCategoryT request_category_type;
	public: typedef IntT int_type;
	public: typedef RealT real_type;


	public: any_model()
	{
		// Empty
	}


	public: template <typename EnterpriseWorkloadModelT>
		any_model(EnterpriseWorkloadModelT const& model)
		: ptr_model_(new model_adaptor<EnterpriseWorkloadModelT>(model))
	{
		// Empty
	}


	public: template <typename EnterpriseWorkloadModelT>
		any_model(::dcs::util::holder<EnterpriseWorkloadModelT> const& wrap_model)
		: ptr_model_(new model_adaptor<EnterpriseWorkloadModelT>(wrap_model.get()))
	{
		// Empty
	}


	public: template <typename EnterpriseWorkloadModelT>
		void model(EnterpriseWorkloadModelT const& model)
	{
		ptr_model_ = new model_adaptor<EnterpriseWorkloadModelT>(model);
	}


	public: template <typename UniformRandomGeneratorT>
		//::std::pair<request_category_type,real_type> generate(UniformRandomGeneratorT& rng) const
		user_request<int_type,real_type> generate(UniformRandomGeneratorT& rng) const
	{
		return ptr_model_->generate(rng);
	}


	private: ::dcs::shared_ptr< base_model<int_type,real_type> > ptr_model_;
};


template <
	typename EnterpriseWorkloadModelT,
	typename EnterpriseWorkloadModelTraitsT=typename ::dcs::type_traits::remove_reference<EnterpriseWorkloadModelT>::type>
struct make_any_model_type
{
    typedef any_model<
				typename EnterpriseWorkloadModelTraitsT::int_type,
				typename EnterpriseWorkloadModelTraitsT::real_type
			> type;
};


namespace detail {

template <
	typename EnterpriseWorkloadModelT,
	typename EnterpriseWorkloadModelTraitsT=typename ::dcs::type_traits::remove_reference<EnterpriseWorkloadModelT>::type
>
struct make_any_model_impl;


template <
	typename EnterpriseWorkloadModelT,
	typename EnterpriseWorkloadModelTraitsT
>
struct make_any_model_impl
{
	typedef typename make_any_model_type<EnterpriseWorkloadModelT,EnterpriseWorkloadModelTraitsT>::type any_model_type;
	static any_model_type apply(EnterpriseWorkloadModelT& model)
	{
		return any_model_type(model);
	}
};


template <
	typename EnterpriseWorkloadModelT,
	typename EnterpriseWorkloadModelTraitsT
>
struct make_any_model_impl<EnterpriseWorkloadModelT&,EnterpriseWorkloadModelTraitsT>
{
	typedef typename make_any_model_type<
						EnterpriseWorkloadModelT,
						EnterpriseWorkloadModelTraitsT
			>::type any_model_type;

	static any_model_type apply(EnterpriseWorkloadModelT& model)
	{
		::dcs::util::holder<EnterpriseWorkloadModelT&> wrap_model(model);
		return any_model_type(wrap_model);
	}
};

} // Namespace detail


template <typename EnterpriseWorkloadModelT>
typename make_any_model_type<EnterpriseWorkloadModelT>::type make_any_model(EnterpriseWorkloadModelT model)
{
	return detail::make_any_model_impl<EnterpriseWorkloadModelT>::apply(model);
}

}}}} // Namespace dcs::perfeval::workload::enterprise


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_ANY_MODEL_HPP
