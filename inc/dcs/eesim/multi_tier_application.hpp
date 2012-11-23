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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_EESIM_MULTI_TIER_APPLICATION_HPP
#define DCS_EESIM_MULTI_TIER_APPLICATION_HPP


#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/application_tier.hpp>
#include <dcs/eesim/base_application_performance_model.hpp>
#include <dcs/eesim/base_application_simulation_model.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/physical_resource_view.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/user_request.hpp>
#include <dcs/exception.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <dcs/memory.hpp>
#include <dcs/perfeval/sla/any_cost_model.hpp>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


namespace dcs { namespace eesim { 

template <typename TraitsT>
class data_center;

template <typename TraitsT>
class base_application_simulation_model;

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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename TraitsT>
class multi_tier_application
{
	private: typedef multi_tier_application<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename traits_type::int_type int_type;
	public: typedef typename traits_type::application_identifier_type identifier_type;
	public: typedef application_tier<traits_type> application_tier_type;
	public: typedef ::dcs::shared_ptr<application_tier_type> application_tier_pointer;
	public: typedef data_center<traits_type> data_center_type;
	public: typedef data_center_type* data_center_pointer;
	public: typedef ::dcs::perfeval::sla::any_cost_model<performance_measure_category,real_type,real_type> sla_cost_model_type;
	public: typedef base_application_performance_model<traits_type> performance_model_type;
	public: typedef ::dcs::shared_ptr<performance_model_type> performance_model_pointer;
	public: typedef base_application_simulation_model<traits_type> simulation_model_type;
	public: typedef ::dcs::shared_ptr<simulation_model_type> simulation_model_pointer;
	public: typedef physical_resource_view<traits_type> reference_physical_resource_type;
	public: typedef ::std::vector<reference_physical_resource_type> reference_physical_resource_container;
	private: typedef ::std::map<
						physical_resource_category,
						reference_physical_resource_type
				> resource_container;
	private: typedef ::std::vector<application_tier_pointer> tier_container;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename des_engine_type::engine_context_type des_engine_context_type;
	private: typedef typename des_engine_type::event_type des_event_type;
	private: typedef ::std::size_t size_type;
	private: typedef user_request<real_type> user_request_type;
	public: typedef ::dcs::des::base_statistic<real_type,uint_type> output_statistic_type;
	public: typedef ::dcs::shared_ptr<output_statistic_type> output_statistic_pointer;


	/// Default constructor.
	public: multi_tier_application()
	: id_(traits_type::invalid_application_id),
	  name_("Unnamed App")
	{
	}


	/// Copy contructor
	private: multi_tier_application(multi_tier_application<traits_type> const& that)
/*
	: id_(traits_type::invalid_application_id),
	  name_(that.name_),
	  tiers_(that.tiers_),
	  sla_cost_model_(that.sla_cost_model_),
	  ref_resources_(that.ref_resources_),
	  ptr_perf_model_(that.ptr_perf_model_->clone()),
	  ptr_sim_model_(that.ptr_perf_model_->clone()),
	  ptr_dc_(that.ptr_dc_)
*/
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy constructor not yet implemented." );
	}


	/// Copy assignment
	private: multi_tier_application<traits_type>& operator=(multi_tier_application<traits_type> const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

/*
		if (&rhs != this)
		{
			id_ = traits_type::invalid_application_id;
			name_ = rhs.name_;
			tiers_ = rhs.tiers_;
			sla_cost_model_ = rhs.sla_cost_model_;
			ref_resources_ = rhs.ref_resources_;
			ptr_perf_model_ = rhs.ptr_perf_model_->clone();
			ptr_sim_model_ = rhs.ptr_sim_model_->clone();
			ptr_dc_ = rhs.ptr_dc_;
		}

		return this;
*/
		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy assignment not yet implemented." );
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


	// BEGIN [sguazt] EXP
	public: void data_centre(data_center_pointer const& ptr_dc)
	{
		ptr_dc_ = ptr_dc;
	}


	public: data_center_type const& data_centre() const
	{
		return *ptr_dc_;
	}


	public: data_center_type& data_centre()
	{
		return *ptr_dc_;
	}
	// END [sguazt] EXP


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
		ptr_sim_model_->application(this);
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


	public: template <typename SlaCostModelT>
		void sla_cost_model(SlaCostModelT const& model)
	{
		sla_cost_model_ = ::dcs::perfeval::sla::make_any_cost_model(model);
	}


	public: sla_cost_model_type& sla_cost_model()
	{
		return sla_cost_model_;
	}


	public: sla_cost_model_type const& sla_cost_model() const
	{
		return sla_cost_model_;
	}


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
		ptr_tier->id(tiers_.size());
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

		reference_resource(reference_physical_resource_type(category, capacity, threshold));
	}


	public: void reference_resource(reference_physical_resource_type const& res)
	{
		ref_resources_[res.category()] = res;
	}


	public: reference_physical_resource_type const& reference_resource(physical_resource_category category) const
	{
		return ref_resources_.at(category);
	}


	public: reference_physical_resource_container reference_resources() const
	{
		reference_physical_resource_container resources;

		typename resource_container::const_iterator end_it = ref_resources_.end();

		for (typename resource_container::const_iterator it = ref_resources_.begin();
			 it != end_it;
			 ++it)
		{
			resources.push_back(it->second);
		}

		return resources;
	}


	public: template <typename VMForwardIterT>
		void start(VMForwardIterT first, VMForwardIterT last)
	{
		DCS_DEBUG_TRACE("BEGIN Starting Application: '" << name_ << "'");

		// check: pointer to simulation model is a valid pointer
		DCS_DEBUG_ASSERT( ptr_sim_model_ );

		ptr_sim_model_->tier_virtual_machines(first, last);
		ptr_sim_model_->start_application();

		DCS_DEBUG_TRACE("END Starting Application: '" << name_ << "'");
	}


	public: void stop()
	{
		DCS_DEBUG_TRACE("BEGIN Stopping Application: '" << name_ << "'");

		// check: pointer to simulation model is a valid pointer
		DCS_DEBUG_ASSERT( ptr_sim_model_ );

		ptr_sim_model_->stop_application();

		DCS_DEBUG_TRACE("END Stopping Application: '" << name_ << "'");
	}


	private: identifier_type id_;
	private: ::std::string name_;
	private: tier_container tiers_;
//	private: workload_model_type workload_;
	private: sla_cost_model_type sla_cost_model_;
//	private: controller_type controller_;
	private: resource_container ref_resources_;
//	private: performance_measure_container ref_measures_;
	private: performance_model_pointer ptr_perf_model_;
	private: simulation_model_pointer ptr_sim_model_;
//	private: des_event_source_pointer ptr_request_evt_src_;
//	private: des_event_source_pointer ptr_response_evt_src_;
//	private: ::std::vector<des_event_source_pointer> ptr_tier_request_evt_srcs_;
//	private: ::std::vector<des_event_source_pointer> ptr_tier_response_evt_srcs_;
//	private: des_event_source_pointer ptr_control_evt_src_;
//	private: uint_type priority_;
	private: data_center_pointer ptr_dc_;
}; // multi_tier_application


template <
	typename CharT,
	typename CharTraitsT,
	typename TraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, multi_tier_application<TraitsT> const& app)
{
	return os << "<"
			  << "ID: " << app.id()
			  << ", Name: " << app.name()
			  << ">";
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_MULTI_TIER_APPLICATION_HPP
