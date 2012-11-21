/**
 * \file dcs/perfeval/workload/enterprise/model_adaptor.hpp
 *
 * \brief Adaptor for enterprise workload model classes.
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

#ifndef DCS_PERFEVAL_WORKLOAD_ENTERPRISE_MODEL_ADAPTOR_HPP
#define DCS_PERFEVAL_WORKLOAD_ENTERPRISE_MODEL_ADAPTOR_HPP


#include <dcs/perfeval/workload/enterprise/base_model.hpp>
#include <dcs/perfeval/workload/enterprise/user_request.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/add_reference.hpp>
#include <utility>


namespace dcs { namespace perfeval { namespace workload { namespace enterprise {

template <
	typename EnterpriseWorkloadModelT,
	typename EnterpriseWorkloadModelTraitsT=typename ::dcs::type_traits::remove_reference<EnterpriseWorkloadModelT>::type
>
class model_adaptor: public base_model<
								typename EnterpriseWorkloadModelTraitsT::int_type,
								typename EnterpriseWorkloadModelTraitsT::real_type
							>
{
	public: typedef typename EnterpriseWorkloadModelTraitsT::int_type int_type;
	public: typedef typename EnterpriseWorkloadModelTraitsT::real_type real_type;
	private: typedef EnterpriseWorkloadModelT model_type;
	public: typedef typename ::dcs::type_traits::add_reference<EnterpriseWorkloadModelT>::type model_reference;
	public: typedef typename ::dcs::type_traits::add_reference<typename ::dcs::type_traits::add_const<EnterpriseWorkloadModelT>::type>::type model_const_reference;


	public: model_adaptor(model_const_reference model)
		: model_(model)
	{
		// empty
	}


	//private: ::std::pair<request_category_type,real_type> do_generate(::dcs::math::random::any_generator<real_type>& rng) const
	private: user_request<int_type,real_type> do_generate(::dcs::math::random::any_generator<real_type>& rng) const
	{
		return model_.generate(rng);
	}


	private: model_type model_;
};

}}}} // Namespace dcs::perfeval::workload::enterprise


#endif // DCS_PERFEVAL_WORKLOAD_ENTERPRISE_MODEL_ADAPTOR_HPP
