/**
 * \file dcs/eesim/multi_tier_application.hpp
 *
 * \brief Model for multi-tier applications.
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

#ifndef DCS_EESIM_MULTI_TIER_APPLICATION_HPP
#define DCS_EESIM_MULTI_TIER_APPLICATION_HPP


#include <cstddef>
#include <dcs/eesim/application_tier.hpp>
#include <dcs/eesim/physical_resource.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/perfeval/sla/any_cost_model.hpp>
#include <dcs/perfeval/workload/enterprise/any_model.hpp>
//#include <dcs/perfeval/workload/generator.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <dcs/memory.hpp>
#include <iostream>
#include <string>
#include <utility>
#include <vector>


namespace dcs { namespace eesim { 

/**
 * \brief Model for multi-tier applications.
 *
 * A multi-tier application model is a tuple:
 * \f[
 *   A = \left\langle
 *         \{T_i\},
 *         W,
 *         \mathbf{y}_r,
 *         C_{SLA},
 *       \right\rangle
 * \f]
 * where:
 * - \f$T_i\f$, for \f$1 \le i \le N_T\f$, is the model for an application tier;
 * - \f$W\f$ is the workload model (e.g., see base_workload_generator).
 * .
 *
 * Note that we assume communication between tier is sequential, that is
 * \f$T_i\f$ is only allowed to send requests to the next tier \f$T_{i+1}\f$ and
 * to accept requests from the previous tier \f$T_{i-1}\f$.
 * Furthermore, the first tier \f$T_1\f$ is called <em>front-end</em> tier,
 * while the last tier \f$T_{N_T}\f$ is called <em>back-end</em> tier.
 *
 * \todo Handle application priority.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <
	typename RequestCategoryT,
	typename TraitsT
>
class multi_tier_application
{
	public: typedef RequestCategoryT request_category_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef application_tier<traits_type> application_tier_type;
	public: typedef ::dcs::shared_ptr<application_tier_type> application_tier_pointer;
	public: typedef physical_resource<traits_type> physical_resource_type;
	public: typedef ::dcs::perfeval::workload::enterprise::any_model<request_category_type,real_type> workload_model_type;
	public: typedef ::dcs::perfeval::sla::any_cost_model<real_type> sla_cost_model_type;
	private: typedef multi_tier_application<request_category_type,traits_type> self_type;
	private: typedef ::std::multimap<
						physical_resource_category,
						physical_resource_type
				> resource_container;
	private: typedef ::std::vector<application_tier_pointer> tier_container;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename des_engine_type::engine_context_type des_engine_context_type;
	private: typedef typename des_engine_type::event_type des_event_type;
	private: typedef typename des_engine_type::event_source_type des_event_source_type;
	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	private: typedef typename traits_type::uniform_random_generator_type uniform_random_generator_type;
	private: typedef ::dcs::control::any_controller<real_type> controller_type;
	private: typedef ::std::size_t size_type;


	private: struct request_state;


	public: explicit multi_tier_application(uint_type priority = 0)
		: name_("Unamed App"),
		  ptr_request_evt_src_(new des_event_source_type()),
		  ptr_response_evt_src_(new des_event_source_type()),
		  priority_(priority)
	{
		init();
	}


	public: void name(::std::string const& value)
	{
		name_ = value;
	}


	public: ::std::string const& name() const
	{
		return name_;
	}


	public: template <typename EnterpriseWorkloadModelT>
		void workload_model(EnterpriseWorkloadModelT const& model)
	{
		workload_ = ::dcs::perfeval::workload::enterprise::make_any_model<EnterpriseWorkloadModelT>(model);
	}


	public: template <typename SlaCostModelT>
		void sla_cost_model(SlaCostModelT const& model)
	{
		sla_cost_ = ::dcs::perfeval::sla::make_any_cost_model(model);
	}


	public: void priority(uint_type value)
	{
		priority_ = value;
	}


	public: uint_type priority() const
	{
		return priority_;
	}


	public: void tier(application_tier_pointer const& ptr_tier)
	{
		ptr_tiers_.push_back(ptr_tier);
		ptr_tier_request_evt_srcs_.push_back(new des_event_source_type());
		ptr_tier_response_evt_srcs_.push_back(new des_event_source_type());
	}


	public: void controller(controller_type const& ctrl)
	{
		controller_ = ctrl;
	}


	public: void add_reference_resource(physical_resource_type const& resource)
	{
		ref_resources_.insert(::std::make_pair(resource.category(), resource));
	}


	public: void run()
	{
		prepare_run();
	}


	private: void init()
	{
		ptr_request_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_request,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		ptr_response_evt_src_->connect(
			::dcs::functional::bind(
				&self_type::process_response,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void prepare_run()
	{
		size_type num_tiers = ptr_tiers_.size();

		for (size_type tier_id = 0; tier_id < num_tiers; ++tier_id)
		{
			ptr_tier_request_evt_srcs_[tier_id]->connect(
				::dcs::functional::bind(
					&self_type::process_request,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					tier_id
				)
			);
			ptr_tier_response_evt_srcs_[tier_id]->connect(
				::dcs::functional::bind(
					&self_type::process_response,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2,
					tier_id
				)
			);
		}

		schedule_request();
	}


	private: void schedule_request()
	{
		::std::pair<request_category_type,real_type> request;

		uniform_random_generator_type& urng = registry<traits_type>::instance().uniform_random_generator();
		des_engine_type& des_engine = registry<traits_type>::instance().des_engine();

		do
		{
			request = workload_.generate(urng);
		}
		while (request.second < 0);

		request_state request_info;
		request_info.category = request.first;
		request_info.iatime = request.second;

		des_engine.schedule_event(
			ptr_request_evt_src_,
			des_engine.sim_time() + request.second,
			request_info
		);
	}


	private: void process_request(des_event_type const& evt, des_engine_context_type& ctx)
	{
		process_request(evt, ctx, 0);
	}


	private: void process_request(des_event_type const& evt, des_engine_context_type& ctx, size_type tier_id)
	{
		uniform_random_generator_type& urng = registry<traits_type>::instance().uniform_random_generator();
		des_engine_type& des_engine = registry<traits_type>::instance().des_engine();

		application_tier_pointer tier = ptr_tiers_.at(tier_id);

		real_type svc_time(0);

		// This loop is needed for coping with distributions
		// that can take non-positive values
		do
		{
			svc_time = ::dcs::math::stats::rand(
							tier->service_distribution(),
							urng
			);
		}
		while (svc_time < 0);

		request_state request_info = evt.template unfolded_state<request_state>();

		if (tier_id == (ptr_tiers_.size()-1))
		{
			// The last (back-end) tier

			des_engine.schedule_event(
				ptr_tier_response_evt_srcs_[tier_id-1],
				des_engine.sim_time() + svc_time,
				request_info
			);
		}
		else
		{
			des_engine.schedule_event(
				ptr_tier_request_evt_srcs_[tier_id+1],
				des_engine.sim_time() + svc_time,
				request_info
			);
		}

		// forward the incoming request to each tier by generating many
		// sub-requests (one for each tier).

//		typename tier_container::iterator it_begin = ptr_tiers_.begin();
//		typename tier_container::iterator it_end = ptr_tiers_.end();
//		typename tier_container::iterator it_prev = ptr_tiers_.begin();
//		for (
//			typename tier_container::iterator it = it_begin;
//			it != it_end;
//			++it
//		) {
//			if (it == it_begin)
//			{
//				it->request_event_source(*ptr_request_evt_src_);
//				ptr_response_evt_src_(it->response_event_source());
//			}
////			else if (it == it_end)
////			{
////				response_event_source(it->response_event_source());
////			}
//			else
//			{
//				it->request_event_source(it_prev->response_event_source());
//				it_prev->response_event_source(it->response_event_source());
//			}
//			it_prev = it;
//		}
	}


	private: void process_response(des_event_type const& evt, des_engine_context_type& ctx)
	{
	}


	private: struct request_state
	{
		real_type iatime;
		request_category_type category; 
	};


	private: ::std::string name_;
	private: tier_container ptr_tiers_;
	private: workload_model_type workload_;
	private: sla_cost_model_type sla_cost_;
	private: controller_type controller_;
	private: resource_container ref_resources_;
	private: des_event_source_pointer ptr_request_evt_src_;
	private: des_event_source_pointer ptr_response_evt_src_;
	private: ::std::vector<des_event_source_pointer> ptr_tier_request_evt_srcs_;
	private: ::std::vector<des_event_source_pointer> ptr_tier_response_evt_srcs_;
	private: uint_type priority_;
};


template <
	typename CharT,
	typename CharTraitsT,
	typename RequestCategoryT,
	typename TraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, multi_tier_application<RequestCategoryT,TraitsT> const& app)
{
	return os << "<"
			  << app.name()
			  << ">";
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_MULTI_TIER_APPLICATION_HPP
