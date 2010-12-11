/**
 * \file dcs/eesim/multi_tier_application.hpp
 *
 * \brief Model for multi-tier applications.
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

#ifndef DCS_EESIM_MULTI_TIER_APPLICATION_HPP
#define DCS_EESIM_MULTI_TIER_APPLICATION_HPP


#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/eesim/application_tier.hpp>
#include <dcs/eesim/base_application_performance_model.hpp>
#include <dcs/eesim/base_application_simulation_model.hpp>
#include <dcs/eesim/performance_measure.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
//#include <dcs/eesim/physical_resource.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/perfeval/sla/any_cost_model.hpp>
//#include <dcs/perfeval/workload/enterprise/any_model.hpp>
//#include <dcs/perfeval/workload/enterprise/user_request.hpp>
//#include <dcs/perfeval/workload/generator.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <dcs/memory.hpp>
#include <iostream>
#include <map>
#include <stdexcept>
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
template <typename TraitsT>
class multi_tier_application
{
	private: struct request_state;
	public: class reference_physical_resource;


	private: typedef multi_tier_application<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename traits_type::int_type int_type;
	public: typedef typename traits_type::application_identifier_type identifier_type;
	public: typedef application_tier<traits_type> application_tier_type;
	public: typedef ::dcs::shared_ptr<application_tier_type> application_tier_pointer;
//	public: typedef physical_resource<traits_type> physical_resource_type;
//	public: typedef ::dcs::shared_ptr<physical_resource_type> physical_resource_pointer;
//	public: typedef ::dcs::perfeval::workload::enterprise::any_model<int_type,real_type> workload_model_type;
	public: typedef ::dcs::perfeval::sla::any_cost_model<real_type> sla_cost_model_type;
	public: typedef base_application_performance_model<traits_type> performance_model_type;
	public: typedef ::dcs::shared_ptr<performance_model_type> performance_model_pointer;
	public: typedef performance_measure<traits_type> performance_measure_type;
	public: typedef base_application_simulation_model<traits_type> simulation_model_type;
	public: typedef ::dcs::shared_ptr<simulation_model_type> simulation_model_pointer;
//	private: typedef ::std::map<
//						physical_resource_category,
//						physical_resource_pointer
//				> resource_container;
	private: typedef ::std::map<
						physical_resource_category,
						reference_physical_resource
				> resource_container;
	private: typedef ::std::vector<application_tier_pointer> tier_container;
	private: typedef ::std::map<
						performance_measure_category,
						performance_measure_type
				> performance_measure_container;
//	private: typedef typename traits_type::des_engine_type des_engine_type;
//	private: typedef typename des_engine_type::engine_context_type des_engine_context_type;
//	private: typedef typename des_engine_type::event_type des_event_type;
//	private: typedef typename des_engine_type::event_source_type des_event_source_type;
//	private: typedef ::dcs::shared_ptr<des_engine_type> des_engine_pointer;
//	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
//	private: typedef typename traits_type::uniform_random_generator_type uniform_random_generator_type;
//	private: typedef ::dcs::shared_ptr<uniform_random_generator_type> uniform_random_generator_pointer;
	private: typedef ::std::size_t size_type;


	public: struct reference_physical_resource
	{
		public: reference_physical_resource()
			: category_(),
			  capacity_(0),
			  threshold_(0)
		{
		}


		public: reference_physical_resource(physical_resource_category category, real_type capacity, real_type threshold)
			: category_(category),
			  capacity_(capacity),
			  threshold_(threshold)
		{
		}


		public: physical_resource_category category() const
		{
			return category_;
		}


		public: real_type capacity() const
		{
			return capacity_;
		}


		public: real_type threshold() const
		{
			return threshold_;
		}


		private: physical_resource_category category_;
		private: real_type capacity_;
		private: real_type threshold_;
	};


	public: multi_tier_application(/*uint_type priority = 0*/)
		: id_(traits_type::invalid_application_id),
		  name_("Unnamed App")
//		  ptr_request_evt_src_(new des_event_source_type()),
//		  ptr_response_evt_src_(new des_event_source_type()),
//		  priority_(priority),
	{
		init();
	}


	public: void id(identifier_type x)
	{
		id_ = x;
	}


	public: identifier_type id() const
	{
		return id_;
	}


	public: void name(::std::string const& value)
	{
		name_ = value;
	}


	public: ::std::string const& name() const
	{
		return name_;
	}


	public: void performance_model(performance_model_pointer const& ptr_model)
	{
		// pre: pointer to performance model must be a valid pointer
		DCS_ASSERT(
			ptr_model,
			throw ::std::invalid_argument("[dcs::eesim::multi_tier_application::performance model] Invalid performance model.")
		);

		ptr_perf_model_ = ptr_model;
	}


	public: performance_model_type& performance_model()
	{
		// check: pointer to performance model must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_perf_model_ );

		return *ptr_perf_model_;
	}


	public: performance_model_type const& performance_model() const
	{
		// check: pointer to performance model must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_perf_model_ );

		return *ptr_perf_model_;
	}


	public: void simulation_model(simulation_model_pointer const& ptr_model)
	{
		// pre: pointer to simulation model must be a valid pointer
		DCS_ASSERT(
			ptr_model,
			throw ::std::invalid_argument("[dcs::eesim::multi_tier_application::simulation model] Invalid simulation model.")
		);

		ptr_sim_model_ = ptr_model;
	}


	public: simulation_model_type& simulation_model()
	{
		// check: pointer to simulation model must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_sim_model_ );

		return *ptr_sim_model_;
	}


	public: simulation_model_type const& simulation_model() const
	{
		// check: pointer to simulation model must be a valid pointer.
		DCS_DEBUG_ASSERT( ptr_sim_model_ );

		return *ptr_sim_model_;
	}


//	public: template <typename EnterpriseWorkloadModelT>
//		void workload_model(EnterpriseWorkloadModelT const& model)
//	{
//		workload_ = ::dcs::perfeval::workload::enterprise::make_any_model<EnterpriseWorkloadModelT>(model);
//	}


	public: template <typename SlaCostModelT>
		void sla_cost_model(SlaCostModelT const& model)
	{
		sla_cost_ = ::dcs::perfeval::sla::make_any_cost_model(model);
	}


	public: sla_cost_model_type& sla_cost_model()
	{
		return sla_cost_;
	}


	public: sla_cost_model_type const& sla_cost_model() const
	{
		return sla_cost_;
	}


//	public: void priority(uint_type value)
//	{
//		priority_ = value;
//	}


//	public: uint_type priority() const
//	{
//		return priority_;
//	}


	public: uint_type num_tiers() const
	{
		return tiers_.size();
	}


	public: void tier(application_tier_pointer const& ptr_tier)
	{
		// pre: pointer to tier must be a valid pointer
		DCS_ASSERT(
			ptr_tier,
			throw ::std::invalid_argument("[dcs::eesim::multi_tier_application::tier] Invalid tier.")
		);

		ptr_tier->application(this);
		tiers_.push_back(ptr_tier);
//		ptr_tier_request_evt_srcs_.push_back(::dcs::make_shared<des_event_source_type>());
//		ptr_tier_response_evt_srcs_.push_back(::dcs::make_shared<des_event_source_type>());
	}


	public: application_tier_pointer const& tier(uint_type id)
	{
		// check: id must be in a valid range
		DCS_DEBUG_ASSERT( id < tiers_.size() );

		return tiers_[id];
	}


	public: application_tier_pointer const& tier(uint_type id) const
	{
		// check: id must be in a valid range
		DCS_DEBUG_ASSERT( id < tiers_.size() );

		return tiers_[id];
	}


	public: ::std::vector<application_tier_pointer> tiers() const
	{
		return tiers_;
	}


//	public: void controller(controller_type const& ctrl)
//	{
//		controller_ = ctrl;
//	}


//	//FIXME: instead of passing a pointer we maybe should pass a const&
//	public: void add_reference_resource(physical_resource_pointer const& ptr_resource)
//	{
//		ref_resources_.insert(::std::make_pair(ptr_resource->category(), ptr_resource));
//	}
//
//
//	public: physical_resource_pointer const& reference_resource(physical_resource_category category) const
//	{
//		typename resource_container::const_iterator it;
//
//		it = ref_resources_.find(category);
//
//		// safety check
//		DCS_DEBUG_ASSERT( it != ref_resources_.end() );
//
//		return it->second;
//	}
//
//
//	public: ::std::vector<physical_resource_pointer> reference_resources() const
//	{
//		::std::vector<physical_resource_pointer> resources;
//
//		typename resource_container::const_iterator end_it = ref_resources_.end();
//
//		for (typename resource_container::const_iterator it = ref_resources_.begin();
//			 it != end_it;
//			 ++it)
//		{
//			resources.push_back(it->second);
//		}
//
//		return resources;
//	}


	public: void reference_resource(physical_resource_category category, real_type capacity, real_type threshold = real_type(1))
	{
		// pre: capacity > 0
		DCS_ASSERT(
			capacity > 0,
			throw ::std::invalid_argument("[dcs::eesim::multi_tier_application::reference_resource] Capacity must a non-negative value.")
		);
		// pre: threshold > 0
		DCS_ASSERT(
			threshold > 0,
			throw ::std::invalid_argument("[dcs::eesim::multi_tier_application::reference_resource] Chreshold must a non-negative value.")
		);

		ref_resources_[category] = reference_physical_resource(category, capacity, threshold);
	}


	public: reference_physical_resource const& reference_resource(physical_resource_category category) const
	{
		return ref_resources_.at(category);
	}


	public: ::std::vector<reference_physical_resource> reference_resources() const
	{
		::std::vector<reference_physical_resource> resources;

		typename resource_container::const_iterator end_it = ref_resources_.end();

		for (typename resource_container::const_iterator it = ref_resources_.begin();
			 it != end_it;
			 ++it)
		{
			resources.push_back(it->second);
		}

		return resources;
	}


	/**
	 * \brief Set the application-level reference performance measures.
	 *
	 * Each element pointed by the iterator must be a pair of
	 *   <category,value>
	 * Each performance measure represents the aggregated performance measure
	 * from each tier.
	 */
	public: void add_reference_performance_measure(performance_measure_type const& measure)
	{
		ref_measures_[measure.category()] = measure;
	}


	public: ::std::vector<performance_measure_type> reference_performance_measures() const
	{
		typedef typename performance_measure_container::const_iterator iterator;

		::std::vector<performance_measure_type> measures;

		iterator end_it = ref_measures_.end();
		for (iterator it = ref_measures_.begin(); it != end_it; ++it)
		{
			measures.push_back(it->second);
		}

		return measures;
	}


	/**
	 * \brief Set the reference performance measures for each application tier.
	 *
	 * The first performance measure is related to the front-end tier, the
	 * second one to the second tier, and so on.
	 */
//	public: template <typename ForwardIteratorT>
//		void reference_tier_performance_measure(application_statistics_category category, ForwardIteratorT first, ForwardIteratorT last)
//	{
//		ref_tier_measures_[category] = ::std::vector<real_type>(first, last);
//	}


//	public: ::std::vector<real_type> reference_tier_performance_measure() const
//	{
//		return ref_tier_measures_;
//	}


	public: void start()
	{
		// check: pointer to simulation model is a valid pointer
		DCS_DEBUG_ASSERT( ptr_sim_model_ );

DCS_DEBUG_TRACE("STARTING APP ---> sim_ptr: " << ptr_sim_model_);//XXX
typedef typename traits_type::uniform_random_generator_type uniform_random_generator_type;
::dcs::shared_ptr<uniform_random_generator_type> ptr_rng = registry<traits_type>::instance().uniform_random_generator_ptr();//XXX
DCS_DEBUG_TRACE("STARTING APP ---> ptr_rng: " << ptr_rng);//XXX
DCS_DEBUG_TRACE("STARTING APP ---> ptr_rng::min: " << ptr_rng->min());//XXX
DCS_DEBUG_TRACE("STARTING APP ---> ptr_rng::max: " << ptr_rng->max());//XXX
		ptr_sim_model_->enable(true);
	}


	public: void stop()
	{
		// check: pointer to simulation model is a valid pointer
		DCS_DEBUG_ASSERT( ptr_sim_model_ );

		ptr_sim_model_->enable(false);
	}


/*
	private: void schedule_request()
	{
		DCS_DEBUG_TRACE("Scheduling Request for Application: " << id_);//XXX
		typedef ::dcs::shared_ptr<des_engine_type> des_engine_pointer;
		typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
		typedef typename traits_type::uniform_random_generator_type uniform_random_generator_type;
		typedef ::dcs::shared_ptr<uniform_random_generator_type> uniform_random_generator_pointer;

		::dcs::perfeval::workload::enterprise::user_request<int_type,real_type> request;

		uniform_random_generator_pointer ptr_urng = registry<traits_type>::instance().uniform_random_generator_ptr();
		des_engine_pointer ptr_des_engine = registry<traits_type>::instance().des_engine_ptr();

		do
		{
			request = workload_.generate(*ptr_urng);
		}
		while (request.interarrival_time() < 0);

		request_state request_info;
		request_info.category = request.category();
		request_info.iatime = request.interarrival_time();

		des_engine->schedule_event(
			ptr_request_evt_src_,
			ptr_des_engine->sim_time() + request.interarrival_time(),
			request_info
		);
	}
*/


	private: void init()
	{
//		ptr_request_evt_src_->connect(
//			::dcs::functional::bind(
//				&self_type::process_request,
//				this,
//				::dcs::functional::placeholders::_1,
//				::dcs::functional::placeholders::_2
//			)
//		);
//		ptr_response_evt_src_->connect(
//			::dcs::functional::bind(
//				&self_type::process_response,
//				this,
//				::dcs::functional::placeholders::_1,
//				::dcs::functional::placeholders::_2
//			)
//		);
//		ptr_control_evt_src_->connect(
//			::dcs::functional::bind(
//				&self_type::process_control,
//				this,
//				::dcs::functional::placeholders::_1,
//				::dcs::functional::placeholders::_2
//			)
//		);
	}


/*
	private: void prepare_run()
	{
		size_type num_tiers = tiers_.size();

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


	private: void process_request(des_event_type const& evt, des_engine_context_type& ctx)
	{
		// Send request to the front-end tier
		process_request(evt, ctx, 0);
	}


	private: void process_request(des_event_type const& evt, des_engine_context_type& ctx, size_type tier_id)
	{
		uniform_random_generator_pointer ptr_urng = registry<traits_type>::instance().uniform_random_generator_ptr();
		des_engine_pointer ptr_des_engine = registry<traits_type>::instance().des_engine_ptr();

		application_tier_pointer tier = tiers_.at(tier_id);

//		// Generate service time for this tier
//		real_type svc_time(0);

//		// This loop is needed for coping with distributions
//		// that can take non-positive values
//		do
//		{
//			svc_time = ::dcs::math::stats::rand(
//					tier->service_distribution(),
//					*ptr_urng
//				);
//		}
//		while (svc_time < 0);
//
//		// Save service time for this tier
//		request_state request_info = evt.template unfolded_state<request_state>();
//		request_info.svctimes[tier_id] = svc_time;

		if (tier_id == (tiers_.size()-1))
		{
			// The last (back-end) tier

			// Generate service time for this tier
			real_type svc_time(0);
			// This loop is needed for coping with distributions
			// that can take non-positive values
			do
			{
				svc_time = ::dcs::math::stats::rand(
						tier->service_distribution(),
						*ptr_urng
					);
			}
			while (svc_time < 0);

			// Save the service time of this tier
			request_info.svctimes.resize(tiers_.size());
			request_info.svctimes[tier_id] = svc_time;

			// Begin the response chain
			des_engine->schedule_event(
				ptr_tier_response_evt_srcs_[tier_id-1],
				ptr_des_engine->sim_time() + svc_time,
				request_info
			);
		}
		else
		{
			// Propagate the request to the next tier
			des_engine->schedule_event(
				ptr_tier_request_evt_srcs_[tier_id+1],
//				ptr_des_engine->sim_time() + svc_time,
				ptr_des_engine->sim_time(),
				request_info
			);
		}

		// forward the incoming request to each tier by generating many
		// sub-requests (one for each tier).

//		typename tier_container::iterator it_begin = tiers_.begin();
//		typename tier_container::iterator it_end = tiers_.end();
//		typename tier_container::iterator it_prev = tiers_.begin();
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
		request_state request_info = evt.template unfolded_state<request_state>();

		DCS_DEBUG_TRACE("REQUEST FINISHED: " << request_info);

		// Schedule a new request
		schedule_request();
	}


	private: void process_response(des_event_type const& evt, des_engine_context_type& ctx, size_type tier_id)
	{
		uniform_random_generator_pointer ptr_urng = registry<traits_type>::instance().uniform_random_generator_ptr();
		des_engine_pointer ptr_des_engine = registry<traits_type>::instance().des_engine_ptr();

		application_tier_pointer tier = tiers_.at(tier_id);

		real_type svc_time(0);

		// This loop is needed for coping with distributions
		// that can take non-positive values
		do
		{
			svc_time = ::dcs::math::stats::rand(
					tier->service_distribution(),
					*ptr_urng
				);
		}
		while (svc_time < 0);

		request_state request_info = evt.template unfolded_state<request_state>();

		// Save the service time of this tier
		request_info.svctimes[tier_id] = svc_time;
		if (tier_id < (tiers_.size()-1))
		{
			// Not in the last tier: the service time of tier k is the sum of service time of tiers k,k+1,..N
			request_info.svctimes[tier_id] += request_info.svctimes[tier_id+1];
		}

		if (tier_id == 0)
		{
			// The first (front-end) tier

			// Finishes the request-reponse process
			ptr_des_engine->schedule_event(
				ptr_response_evt_src_,
				ptr_des_engine->sim_time() + svc_time,
				request_info
			);
		}
		else
		{
			// Propagate the response to the previous tier
			ptr_des_engine->schedule_event(
				ptr_tier_response_evt_srcs_[tier_id-1],
				ptr_des_engine->sim_time() + svc_time,
				request_info
			);
		}

	}


//	private: void process_control(des_event_type const& evt, des_engine_context_type& ctx)
//	{
//	}


	private: struct request_state
	{
		real_type iatime; ///< Arrival time to the first tier.
		int_type category; ///< Request category.
		::std::vector<real_type> svctimes; ///< Service time at each tier (i-th element refers to the i-th tier).

		template <typename CharT, typename CharTraitsT>
		friend ::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, request_state const& state)
		{
			os << "<"
			   << "Arrival: " << state.iatime
			   << ", Category: " << state.category
			   << ", Service Times: ";

			for (size_type i = 0; i < state.svctimes.size(); ++i)
			{
				if (i > 0)
				{
					os << ", ";
				}
				os << state.svctimes[i];
			}

			os << ">";

			return os;
		}
	};
*/


	private: identifier_type id_;
	private: ::std::string name_;
	private: tier_container tiers_;
//	private: workload_model_type workload_;
	private: sla_cost_model_type sla_cost_;
//	private: controller_type controller_;
	private: resource_container ref_resources_;
	private: performance_measure_container ref_measures_;
	private: performance_model_pointer ptr_perf_model_;
	private: simulation_model_pointer ptr_sim_model_;
//	private: des_event_source_pointer ptr_request_evt_src_;
//	private: des_event_source_pointer ptr_response_evt_src_;
//	private: ::std::vector<des_event_source_pointer> ptr_tier_request_evt_srcs_;
//	private: ::std::vector<des_event_source_pointer> ptr_tier_response_evt_srcs_;
//	private: des_event_source_pointer ptr_control_evt_src_;
//	private: uint_type priority_;
};


template <
	typename CharT,
	typename CharTraitsT,
	typename TraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, multi_tier_application<TraitsT> const& app)
{
	return os << "<"
			  << app.name()
			  << ">";
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_MULTI_TIER_APPLICATION_HPP
