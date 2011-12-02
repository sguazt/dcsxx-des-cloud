/**
 * \file src/dcs/eesim/default_physical_machine_simulation_model.hpp
 *
 * \brief Simulation model class for physical machines.
 *
 * Copyright (C) 2009-2011  Distributed Computing System (DCS) Group, Computer
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

#ifndef DCS_EESIM_DEFAULT_PHYSICAL_MACHINE_SIMULATION_MODEL_HPP
#define DCS_EESIM_DEFAULT_PHYSICAL_MACHINE_SIMULATION_MODEL_HPP


#include <algorithm>
#include <boost/icl/concept/interval.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/des/weighted_mean_estimator.hpp>
#include <dcs/math/traits/float.hpp>
#include <dcs/eesim/base_physical_machine_simulation_model.hpp>
#include <dcs/eesim/logging.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/power_status.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/resource_utilization_profile.hpp>
#include <dcs/eesim/user_request.hpp>
#include <dcs/exception.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class default_physical_machine_simulation_model: public base_physical_machine_simulation_model<TraitsT>
{
	private: typedef base_physical_machine_simulation_model<TraitsT> base_type;
	private: typedef default_physical_machine_simulation_model<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	private: typedef typename traits_type::real_type real_type;
	private: typedef typename traits_type::uint_type uint_type;
	private: typedef typename base_type::output_statistic_type output_statistic_type;
	private: typedef typename base_type::output_statistic_pointer output_statistic_pointer;
	private: typedef typename base_type::des_event_source_type des_event_source_type;
	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	private: typedef typename base_type::des_event_type des_event_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	//FIXME: statistic type (mean estimator) is hard-coded
	private: typedef ::dcs::des::mean_estimator<real_type,uint_type> mean_estimator_statistic_type;
	private: typedef ::dcs::des::weighted_mean_estimator<real_type,uint_type> weighted_mean_estimator_statistic_type;
	private: typedef registry<traits_type> registry_type;
	private: typedef typename base_type::physical_machine_type physical_machine_type;
	private: typedef typename base_type::virtual_machine_type virtual_machine_type;
	private: typedef typename base_type::virtual_machine_pointer virtual_machine_pointer;
	//private: typedef user_request<traits_type> user_request_type;
	private: typedef resource_utilization_profile<traits_type> utilization_profile_type;
	private: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
	private: typedef ::std::map< virtual_machine_identifier_type, ::std::vector< ::std::pair<real_type,real_type> > > virtual_machine_hosting_time_map;
	private: typedef ::std::map< virtual_machine_identifier_type, ::std::map<physical_resource_category,real_type> > virtual_machine_share_time_map;


	private: static const ::std::string poweron_event_source_name;
	private: static const ::std::string poweroff_event_source_name;
	private: static const ::std::string vm_poweron_event_source_name;
	private: static const ::std::string vm_poweroff_event_source_name;
	private: static const ::std::string vm_suspend_event_source_name;
	private: static const ::std::string vm_resume_event_source_name;
	private: static const ::std::string vm_migration_event_source_name;


	public: default_physical_machine_simulation_model()
	: base_type(),
	  pwr_state_(powered_off_power_status),
//	  energy_(0),
	  uptime_(0),
	  last_pwron_time_(0),
	  ptr_pwron_evt_src_(new des_event_source_type(poweron_event_source_name)),
	  ptr_pwroff_evt_src_(new des_event_source_type(poweroff_event_source_name)),
	  ptr_vm_pwron_evt_src_(new des_event_source_type(vm_poweron_event_source_name)),
	  ptr_vm_pwroff_evt_src_(new des_event_source_type(vm_poweroff_event_source_name)),
	  ptr_vm_suspend_evt_src_(new des_event_source_type(vm_suspend_event_source_name)),
	  ptr_vm_resume_evt_src_(new des_event_source_type(vm_resume_event_source_name)),
	  ptr_vm_migr_evt_src_(new des_event_source_type(vm_migration_event_source_name)),
	  ptr_energy_stat_(new mean_estimator_statistic_type()), //FIXME: statistic type (mean) is hard-coded
	  ptr_uptime_stat_(new mean_estimator_statistic_type()), //FIXME: statistic type (mean) is hard-coded
	  ptr_util_stat_(new mean_estimator_statistic_type()), //FIXME: statistic type (mean) is hard-coded
	  ptr_share_stat_(registry_type::instance().des_engine().make_analyzable_statistic(weighted_mean_estimator_statistic_type())), //FIXME: statistic type (mean) is hard-coded
#ifdef DDCS_EESIM_EXP_PHYSICAL_MACHINES_AUTO_POWER_OFF
	  last_share_upd_time_(0),
	  can_pm_auto_pow_off_(true)
#else // DDCS_EESIM_EXP_PHYSICAL_MACHINES_AUTO_POWER_OFF
	  last_share_upd_time_(0)
#endif // DDCS_EESIM_EXP_PHYSICAL_MACHINES_AUTO_POWER_OFF
	{
		init();
	}


	/// Copy constructor.
	private: default_physical_machine_simulation_model(default_physical_machine_simulation_model const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: default_physical_machine_simulation_model& operator=(default_physical_machine_simulation_model const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
	}


	/// The destructor
	public: ~default_physical_machine_simulation_model()
	{
		finit();
	}


	private: void init()
	{
//		if (this->enabled())
//		{
			connect_to_event_sources();
//		}
	}


	private: void finit()
	{
//		if (this->enabled())
//		{
			disconnect_from_event_sources();
//		}
	}


	private: void connect_to_event_sources()
	{
		// Connect to foreign event sources
		registry_type& reg(registry_type::instance());

		reg.des_engine().begin_of_sim_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_begin_of_sim,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		reg.des_engine().system_initialization_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_sys_init,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		reg.des_engine().system_finalization_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_sys_finit,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void disconnect_from_event_sources()
	{
		// Disconnect from foreign event sources
		registry_type& reg(registry_type::instance());

		reg.des_engine().begin_of_sim_event_source().disconnect(
			::dcs::functional::bind(
				&self_type::process_begin_of_sim,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		reg.des_engine().system_initialization_event_source().disconnect(
			::dcs::functional::bind(
				&self_type::process_sys_init,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
		reg.des_engine().system_finalization_event_source().disconnect(
			::dcs::functional::bind(
				&self_type::process_sys_finit,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


//FIXME: this may be wrong...review
/*
	private: void update_experiment_stats_old(des_engine_context_type& ctx)
	{
//		typedef typename physical_machine_type::vmm_type vmm_type;
//		typedef typename vmm_type::virtual_machine_container vm_container;
//		typedef typename vm_container::const_iterator vm_iterator;
//		typedef typename vmm_type::virtual_machine_type vm_type;
//		typedef typename vm_container::value_type vm_pointer;
//		typedef typename vm_type::application_tier_type application_tier_type;
//		typedef typename vm_type::resource_share_container resource_share_container;
//		typedef typename resource_share_container::const_iterator resource_share_iterator;

//		// Update consumed energy: compute the energy consumed by each VM
//		// running on this machine without considering the energy consumed when
//		// the machine is idle; this last energy is first subtracted from the
//		// energy returned by the model and then added at the end of the
//		// computation.
//		//real_type energy(0);
//		//real_type idle_energy(this->machine().consumed_energy(0));
//		vm_container vms(this->machine().vmm().virtual_machines());
//		vm_iterator vm_end_it(vms.end());
//		real_type busy_capacity(0);
//		for (vm_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
//		{
//			vm_pointer ptr_vm(*vm_it);
//
//			application_tier_type const& ref_tier(ptr_vm->guest_system());
//
//			busy_capacity += ref_tier.application().simulation_model().tier_busy_capacity(ref_tier.id());
////			real_type busy_time;
////			busy_time = ref_tier.application().simulation_model().actual_tier_busy_time(ref_tier.id());
////
////			if (busy_time > 0)
////			{
////				resource_share_container shares(ptr_vm->resource_shares());
////				resource_share_iterator res_end_it(shares.end());
////				for (resource_share_iterator res_it = shares.begin(); res_it != res_end_it; ++res_it)
////				{
////					physical_resource_category category(res_it->first);
////					real_type share(res_it->second);
////
////					energy += busy_time * (this->machine().resource(category)->consumed_energy(share) - idle_energy);
////				}
////			}
//		}
////		energy_ += energy + idle_energy;
//		if (busy_capacity > 0)
//		{
//			typedef typename physical_machine_type::resource_pointer resource_pointer;
//			typedef ::std::vector<resource_pointer> resource_container;
//			typedef typename resource_container::const_iterator resource_iterator;
//
//			resource_container resources(this->machine().resources());
//			resource_iterator res_end_it(resources.end());
//			for (resource_iterator it = resources.begin(); it != res_end_it; ++it)
//			{
//				resource_pointer ptr_res(*it);
//
//				energy_ += ptr_res->consumed_energy(busy_capacity)-ptr_res->consumed_energy(0)
//						+  ptr_res->consumed_energy(0)*static_cast<real_type>(ctx.simulated_time() - last_pwron_time_);
//			}
//		}

		// Update uptime
		uptime_ += ctx.simulated_time() - last_pwron_time_;
	}
*/


/*
	private: void update_experiment_stats()
	{
		registry_type& reg(registry_type::instance());

		real_type cur_time(reg.des_engine().simulated_time());

		// Update uptime

		uptime_ += cur_time - last_pwron_time_;

		// Update resource utilization profile

		typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;
		typedef typename virtual_machine_container::const_iterator vm_iterator;
		typedef user_request<traits_type> user_request_type;
		typedef ::std::vector<user_request_type> request_container;
		typedef typename request_container::const_iterator request_iterator;
		typedef typename utilization_profile_type::const_iterator profile_iterator;
		typedef typename utilization_profile_type::time_interval_type time_interval_type;

		virtual_machine_container active_vms(this->machine().vmm().virtual_machines(powered_on_power_status));
		time_interval_type last_uptime(utilization_profile_type::make_time_interval(last_pwron_time_, cur_time));
		vm_iterator vm_end_it(active_vms.end());
		for (vm_iterator vm_it = active_vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_pointer ptr_vm(*vm_it);

			// check: valid VM pointer (paranoid)
			DCS_DEBUG_ASSERT( ptr_vm );

			request_container reqs(ptr_vm->guest_system().application().simulation_model().tier_in_service_requests(ptr_vm->guest_system().id()));
			request_iterator req_end_it(reqs.end());
			for (request_iterator req_it = reqs.begin(); req_it != req_end_it; ++req_it)
			{
				user_request_type const& req(*req_it);
				//FIXME: CPU resource category is hard-coded
				physical_resource_category category(cpu_resource_category);

				::std::vector<utilization_profile_type> profiles;
				profiles = req.tier_utilization_profiles(ptr_vm->guest_system().id(), category);
				if (profiles.size() > 0)
				{
					// The last profiles refers to the last visit (at this tier)
					//utilization_profile_type const& profile(profiles.back());
					utilization_profile_type profile(make_profile_from_intersection(profiles.back(), last_uptime));
					profile_iterator profile_end_it(profile.end());
					for (profile_iterator profile_it = profile.begin(); profile_it != profile_end_it; ++profile_it)
					{
						res_profile_map_[category](*profile_it);
					}
				}
			}
		}
	}
*/


	private: void update_utilization_profile(virtual_machine_pointer const& ptr_vm)
	{
		DCS_DEBUG_TRACE("(" << this << ") BEGIN Updating Utilization Profile (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");

		DCS_DEBUG_ASSERT( ptr_vm );

		DCS_DEBUG_ASSERT( vm_host_time_map_.count(ptr_vm->id()) > 0 );
		DCS_DEBUG_ASSERT( vm_host_time_map_.at(ptr_vm->id()).size() > 0 );

		typedef user_request<traits_type> user_request_type;
		typedef ::std::vector<user_request_type> request_container;
		typedef typename request_container::const_iterator request_iterator;
		typedef typename utilization_profile_type::const_iterator profile_iterator;
		typedef typename utilization_profile_type::time_interval_type time_interval_type;

		registry_type& reg(registry_type::instance());

		real_type cur_time(reg.des_engine().simulated_time());

//		time_interval_type host_time(utilization_profile_type::make_time_interval(vm_host_time_map.at(ptr_vm->id()).back().first, vm_host_time_map.at(ptr_vm->id()).back().second));
		time_interval_type host_time(utilization_profile_type::make_time_interval(vm_host_time_map_.at(ptr_vm->id()).back().first, ::std::min(vm_host_time_map_.at(ptr_vm->id()).back().second, cur_time)));
		request_container reqs(ptr_vm->guest_system().application().simulation_model().tier_in_service_requests(ptr_vm->guest_system().id()));
		request_iterator req_end_it(reqs.end());
		for (request_iterator req_it = reqs.begin(); req_it != req_end_it; ++req_it)
		{
			user_request_type const& req(*req_it);

			update_utilization_profile(ptr_vm, req);
/*
			//FIXME: CPU resource category is hard-coded
			physical_resource_category category(cpu_resource_category);

			::std::vector<utilization_profile_type> profiles;
			profiles = req.tier_utilization_profiles(ptr_vm->guest_system().id(), category);
			if (profiles.size() > 0)
			{
				// The last profiles refers to the last visit (at this tier)
				//utilization_profile_type const& profile(profiles.back());
				utilization_profile_type profile(make_profile_from_intersection(profiles.back(), host_time));
				profile_iterator profile_end_it(profile.end());
				for (profile_iterator profile_it = profile.begin(); profile_it != profile_end_it; ++profile_it)
				{
					res_profile_map_[category](*profile_it);
				}
			}
*/
		}

		DCS_DEBUG_TRACE("(" << this << ") END Updating Utilization Profile (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");
	}


	private: void update_utilization_profile(virtual_machine_pointer const& ptr_vm, user_request<traits_type> const& req)
	{
		DCS_DEBUG_TRACE("(" << this << ") BEGIN Updating Utilization Profile (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");

		DCS_DEBUG_ASSERT( ptr_vm );

//::std::cerr << "[default_physical_machine_simulation_model] (" << this << ") Update Utilization Profile - PM: " << this->machine() << " - VM: " << *ptr_vm << ::std::endl;//XXX
		DCS_DEBUG_ASSERT( vm_host_time_map_.count(ptr_vm->id()) > 0 );
		DCS_DEBUG_ASSERT( vm_host_time_map_.at(ptr_vm->id()).size() > 0 );

		typedef typename utilization_profile_type::const_iterator profile_iterator;
		typedef typename utilization_profile_type::time_interval_type time_interval_type;

		registry_type& reg(registry_type::instance());

		real_type cur_time(reg.des_engine().simulated_time());

//		time_interval_type host_time(utilization_profile_type::make_time_interval(vm_host_time_map.at(ptr_vm->id()).back().first, vm_host_time_map.at(ptr_vm->id()).back().second));
		time_interval_type host_time(utilization_profile_type::make_time_interval(vm_host_time_map_.at(ptr_vm->id()).back().first, ::std::min(vm_host_time_map_.at(ptr_vm->id()).back().second, cur_time)));
		//FIXME: CPU resource category is hard-coded
		physical_resource_category category(cpu_resource_category);

		::std::vector<utilization_profile_type> profiles;
		profiles = req.tier_utilization_profiles(ptr_vm->guest_system().id(), category);
		if (profiles.size() > 0)
		{
			// The last profiles refers to the last visit (at this tier)
			//utilization_profile_type const& profile(profiles.back());
			utilization_profile_type profile(make_profile_from_intersection(profiles.back(), host_time));
			profile_iterator profile_end_it(profile.end());
			for (profile_iterator profile_it = profile.begin(); profile_it != profile_end_it; ++profile_it)
			{
				res_profile_map_[category](*profile_it);
			}
		}

		DCS_DEBUG_TRACE("(" << this << ") END Updating Utilization Profile (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");
	}


	private: void update_share_profile(physical_resource_category category)
	{
		DCS_DEBUG_TRACE("(" << this << ") BEGIN Updating Share Profile (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");

		//FIXME: CPU resource category is hard-coded
		DCS_DEBUG_ASSERT( category == cpu_resource_category );

		typedef typename physical_machine_type::vmm_type vmm_type;
		typedef typename vmm_type::virtual_machine_container vm_container;
		typedef typename vm_container::const_iterator vm_iterator;

		real_type cur_time(registry_type::instance().des_engine().simulated_time());

		// paranoid-check: consistency
		DCS_DEBUG_ASSERT( cur_time >= last_share_upd_time_ );

		// Safety check to prevent NaNs
		if (::dcs::math::float_traits<real_type>::definitely_greater(cur_time, last_share_upd_time_))
		{
			real_type aggr_share(0);

			vm_container vms(this->machine().vmm().virtual_machines(powered_on_power_status));
			vm_iterator vm_end_it(vms.end());
			for (vm_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
			{
				virtual_machine_pointer ptr_vm(*vm_it);

				// paranoid-check: null
				DCS_DEBUG_ASSERT( ptr_vm );

				aggr_share += ptr_vm->resource_share(category);
			}

			(*ptr_share_stat_)(aggr_share, cur_time-last_share_upd_time_);
//			(*ptr_share_stat_)(aggr_share);
		}

		last_share_upd_time_ = cur_time;
	}


	private: void update_share_profiles()
	{
		//FIXME: CPU resource category is hard-coded
		update_share_profile(cpu_resource_category);
	}


/*
	private: void update_resource_share_stat(physical_resource_category category)
	{
		//FIXME: CPU resource category is hard-coded
		DCS_DEBUG_ASSERT( category == cpu_resource_category );

		typedef typename physical_machine_type::vmm_type vmm_type;
		typedef typename vmm_type::virtual_machine_container vm_container;
		typedef typename vm_container::const_iterator vm_iterator;

		real_type aggr_share(0);

::std::cerr << "[default_physical_machine_simulation] BEGIN Update Share Stats" << ::std::endl;///XXX
::std::cerr << "[default_physical_machine_simulation] Old Share Stat: " << ptr_share_stat_->estimate() << ::std::endl;///XXX
		vm_container vms(this->machine().vmm().virtual_machines(powered_on_power_status));
		vm_iterator vm_end_it(vms.end());
		for (vm_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_pointer ptr_vm(*vm_it);
::std::cerr << "[default_physical_machine_simulation] VM: " << *ptr_vm << " --> " << ptr_vm->resource_share(category) << ::std::endl;///XXX

			// paranoid-check: null
			DCS_DEBUG_ASSERT( ptr_vm );

			aggr_share += ptr_vm->resource_share(category);
		}

		real_type cur_time(registry_type::instance().des_engine().simulated_time());

		// paranoid-check: consistency
		DCS_DEBUG_ASSERT( cur_time >= share_stat_upd_time_ );

::std::cerr << "[default_physical_machine_simulation] Aggregate Share: " << aggr_share << " - Weight: " << (cur_time-share_stat_upd_time_) << ::std::endl;///XXX
		// Safety check to prevent NaNs
		if (::dcs::math::float_traits<real_type>::definitely_greater(cur_time, share_stat_upd_time_) > 0)
		{
			(*ptr_share_stat_)(aggr_share, cur_time-share_stat_upd_time_);
//			(*ptr_share_stat_)(aggr_share);

		}
		share_stat_upd_time_ = cur_time;
::std::cerr << "[default_physical_machine_simulation] New Share Stat: " << ptr_share_stat_->estimate() << ::std::endl;///XXX
::std::cerr << "[default_physical_machine_simulation] END Update Share Stats" << ::std::endl;///XXX
	}


	private: void update_resource_share_stats()
	{
//		typedef typename resource_statistic_map::const_iterator iterator;
//
//		iterator end_it(share_stat_map_.end());
//		for (iterator it = share_stat_map_.begin(); it != end_it; ++it)
//		{
//			physical_resource_category category(it->first);
//
//			update_resource_share_stat(category);
//		}

		//FIXME: CPU resource category is hard-coded
		update_resource_share_stat(cpu_resource_category);
	}


	private: void update_vm_share_time(real_type time)
	{
		typedef typename physical_machine_type::vmm_type vmm_type;
		typedef typename vmm_type::virtual_machine_container vm_container;
		typedef typename vm_container::const_iterator vm_iterator;

::std::cerr << "[default_physical_machine_simulation] BEGIN Update Share Time: " << time << ::std::endl;///XXX
		vm_container vms(this->machine().vmm().virtual_machines(powered_on_power_status));
		vm_iterator vm_end_it(vms.end());
		for (vm_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_pointer ptr_vm(*vm_it);
::std::cerr << "[default_physical_machine_simulation] VM: " << *ptr_vm << " --> " << ptr_vm->resource_share(cpu_resource_category) << ::std::endl;///XXX

			// paranoid-check: null
			DCS_DEBUG_ASSERT( ptr_vm );

			vm_share_time_map_[ptr_vm->id()][cpu_resource_category] = time;
		}
::std::cerr << "[default_physical_machine_simulation] END Update Share Time: " << time << ::std::endl;///XXX
	}
*/


	//@{ Interface Member Functions

	protected: void do_enable(bool flag)
	{
		base_type::do_enable(flag);

		ptr_energy_stat_->enable(flag);
		ptr_uptime_stat_->enable(flag);
		ptr_util_stat_->enable(flag);
		ptr_share_stat_->enable(flag);

		// Update this time so that we don't take into account inactive periods
		if (flag ^ this->enabled())
		{
			if (flag)
			{
				last_share_upd_time_ = registry_type::instance().des_engine().simulated_time();
			}
			else
			{
				update_share_profiles();
			}
		}

		ptr_pwron_evt_src_->enable(flag);
		ptr_pwroff_evt_src_->enable(flag);
		ptr_vm_pwron_evt_src_->enable(flag);
		ptr_vm_pwroff_evt_src_->enable(flag);
		ptr_vm_migr_evt_src_->enable(flag);

//		if (flag)
//		{
//			if (!this->enabled())
//			{
//				connect_to_event_sources();
//			}
//		}
//		else
//		{
//			if (this->enabled())
//			{
//				disconnect_from_event_sources();
//			}
//		}
	}


	private: power_status do_power_state() const
	{
		return pwr_state_;
	}


	private: void do_power_on()
	{
		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Power-On physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");
//::std::cerr << "[default_physical_machine_simulation] (" << this << ") BEGIN Do Power-On physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")" << ::std::endl;//XXX

		if (pwr_state_ == powered_off_power_status)
		{
			pwr_state_ = powered_on_power_status;

			registry_type& reg(registry_type::instance());

			real_type cur_time(reg.des_engine().simulated_time());

			// Update info for uptime and other stats
			last_pwron_time_ = cur_time;
			last_share_upd_time_ = cur_time;

			// Fire the power-on event
			reg.des_engine().schedule_event(
					ptr_pwron_evt_src_,
					cur_time
				);
		}
		else
		{
			log_warn(DCS_EESIM_LOGGING_AT, "Cannot power-on a non powered-off physical machine.");
		}

//::std::cerr << "[default_physical_machine_simulation] (" << this << ") END Do Power-On physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")" << ::std::endl;//XXX
		DCS_DEBUG_TRACE("(" << this << ") END Do Power-On physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");
	}


	private: void do_power_off()
	{
		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Power-Off physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");
//::std::cerr << "[default_physical_machine_simulation] (" << this << ") BEGIN Do Power-Off physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")" << ::std::endl;//XXX

		if (pwr_state_ != powered_off_power_status)
		{
			// Power-off hosted VMs
			typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;
			typedef typename virtual_machine_container::iterator virtual_machine_iterator;
#ifdef DDCS_EESIM_EXP_PHYSICAL_MACHINES_AUTO_POWER_OFF
	  		can_pm_auto_pow_off_ = false;
#endif // DDCS_EESIM_EXP_PHYSICAL_MACHINES_AUTO_POWER_OFF
			virtual_machine_container vms(this->machine().vmm().virtual_machines());
			virtual_machine_iterator vm_end_it(vms.end());
			for (virtual_machine_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
			{
				virtual_machine_pointer ptr_vm(*vm_it);

				this->machine().vmm().power_off(ptr_vm);
			}

			pwr_state_ = powered_off_power_status;

			registry_type& reg(registry_type::instance());

			real_type cur_time(reg.des_engine().simulated_time());

			// Update the uptime
//::std::cerr << "Update UPTIME - old: " << uptime_ << " - new: " << (uptime_+(cur_time-last_pwron_time_)) << ::std::endl;//XXX
			uptime_ += cur_time - last_pwron_time_;

//			update_resource_share_stats();

			// Fire the power-off event
			reg.des_engine().schedule_event(
					ptr_pwroff_evt_src_,
					cur_time
				);
#ifdef DDCS_EESIM_EXP_PHYSICAL_MACHINES_AUTO_POWER_OFF
	  		can_pm_auto_pow_off_ = true;
#endif // DDCS_EESIM_EXP_PHYSICAL_MACHINES_AUTO_POWER_OFF
		}
		else
		{
			log_warn(DCS_EESIM_LOGGING_AT, "Cannot power-off an already powered-off physical machine.");
		}

//::std::cerr << "[default_physical_machine_simulation] (" << this << ") END Do Power-Off physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")" << ::std::endl;//XXX
		DCS_DEBUG_TRACE("(" << this << ") END Do Power-Off physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");
	}


	private: power_status do_vm_power_state(virtual_machine_pointer const& ptr_vm) const
	{
		DCS_DEBUG_ASSERT( ptr_vm );

		return ptr_vm->power_state();
	}


	private: void do_vm_power_on(virtual_machine_pointer const& ptr_vm)
	{
		DCS_DEBUG_ASSERT( ptr_vm );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Power-On virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");
//::std::cerr << "[default_physical_machine_simulation_model] (" << this << ") BEGIN Do Power-On virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")" << ::std::endl;//XXX

		if (ptr_vm->power_state() == powered_off_power_status)
		{
			ptr_vm->power_on();

			registry_type& reg(registry_type::instance());

			real_type cur_time(reg.des_engine().simulated_time());

			vm_host_time_map_[ptr_vm->id()].push_back(::std::make_pair(cur_time, ::std::numeric_limits<real_type>::infinity()));

			reg.des_engine().schedule_event(
					ptr_vm_pwron_evt_src_,
					cur_time,
					ptr_vm
				);

			ptr_vm->guest_system().application().simulation_model().request_tier_service_event_source(ptr_vm->guest_system().id()).connect(
					::dcs::functional::bind(
							&self_type::process_vm_request_service,
							this,
							::dcs::functional::placeholders::_1,
							::dcs::functional::placeholders::_2,
							ptr_vm
						)
				);
		}
		else
		{
			log_warn(DCS_EESIM_LOGGING_AT, "Cannot power-on a non powered-off virtual machine.");
		}

//::std::cerr << "[default_physical_machine_simulation_model] (" << this << ") END Do Power-On virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")" << ::std::endl;//XXX
		DCS_DEBUG_TRACE("(" << this << ") END Do Power-On virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");
	}


	private: void do_vm_power_off(virtual_machine_pointer const& ptr_vm)
	{
		DCS_DEBUG_ASSERT( ptr_vm );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Power-Off virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");
//::std::cerr << "[default_physical_machine_simulation_model] (" << this << ") BEGIN Do Power-Off virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")" << ::std::endl;///XXX

		if (ptr_vm->power_state() != powered_off_power_status)
		{
			registry_type& reg(registry_type::instance());

			real_type cur_time(reg.des_engine().simulated_time());

			DCS_DEBUG_ASSERT( vm_host_time_map_.count(ptr_vm->id()) > 0 );
			DCS_DEBUG_ASSERT( vm_host_time_map_.at(ptr_vm->id()).size() > 0 );

			vm_host_time_map_[ptr_vm->id()].back().second = cur_time;

			update_utilization_profile(ptr_vm);
			update_share_profiles();

			ptr_vm->power_off();

			reg.des_engine().schedule_event(
					ptr_vm_pwroff_evt_src_,
					cur_time,
					ptr_vm
				);

			ptr_vm->guest_system().application().simulation_model().request_tier_service_event_source(ptr_vm->guest_system().id()).disconnect(
					::dcs::functional::bind(
							&self_type::process_vm_request_service,
							this,
							::dcs::functional::placeholders::_1,
							::dcs::functional::placeholders::_2,
							ptr_vm
						)
				);
		}
		else
		{
			log_warn(DCS_EESIM_LOGGING_AT, "Cannot power-off an already powered-off virtual machine.");
		}

#ifdef DDCS_EESIM_EXP_PHYSICAL_MACHINES_AUTO_POWER_OFF
			if (can_pm_auto_pow_off_
				&&
				this->machine().vmm().virtual_machines(powered_on_power_status).size() == 0)
			{
::std::cerr << "[default_physical_machine_simulation_model] Auto-powering off machine" << this->machine() << ::std::endl;//XXX
				this->machine().power_off();
			}
#endif // DDCS_EESIM_EXP_PHYSICAL_MACHINES_AUTO_POWER_OFF
//::std::cerr << "(" << this << ") END Do Power-Off virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")" << ::std::endl;///XXX
		DCS_DEBUG_TRACE("[default_physical_machine_simulation_model] (" << this << ") END Do Power-Off virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");
	}


	private: void do_vm_suspend(virtual_machine_pointer const& ptr_vm)
	{
		DCS_DEBUG_ASSERT( ptr_vm );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Suspend virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");

		if (ptr_vm->power_state() == powered_on_power_status)
		{
			registry_type& reg(registry_type::instance());

			real_type cur_time(reg.des_engine().simulated_time());

			DCS_DEBUG_ASSERT( vm_host_time_map_.count(ptr_vm->id()) > 0 );
			DCS_DEBUG_ASSERT( vm_host_time_map_.at(ptr_vm->id()).size() > 0 );

			vm_host_time_map_[ptr_vm->id()].back().second = cur_time;

			update_utilization_profile(ptr_vm);
			update_share_profiles();

			ptr_vm->suspend();

			reg.des_engine().schedule_event(
					ptr_vm_suspend_evt_src_,
					cur_time,
					ptr_vm
				);

			ptr_vm->guest_system().application().simulation_model().request_tier_service_event_source(ptr_vm->guest_system().id()).disconnect(
					::dcs::functional::bind(
							&self_type::process_vm_request_service,
							this,
							::dcs::functional::placeholders::_1,
							::dcs::functional::placeholders::_2,
							ptr_vm
						)
				);
		}
		else
		{
			log_warn(DCS_EESIM_LOGGING_AT, "Cannot suspend an non powered-on virtual machine.");
		}

		DCS_DEBUG_TRACE("(" << this << ") END Do Suspend virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");
	}


	private: void do_vm_resume(virtual_machine_pointer const& ptr_vm)
	{
		DCS_DEBUG_ASSERT( ptr_vm );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Resume virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");

		if (ptr_vm->power_state() == suspended_power_status)
		{
			registry_type& reg(registry_type::instance());

			real_type cur_time(reg.des_engine().simulated_time());

			DCS_DEBUG_ASSERT( vm_host_time_map_.count(ptr_vm->id()) > 0 );
			DCS_DEBUG_ASSERT( vm_host_time_map_.at(ptr_vm->id()).size() > 0 );

			vm_host_time_map_[ptr_vm->id()].push_back(::std::make_pair(cur_time, ::std::numeric_limits<real_type>::infinity()));

			ptr_vm->resume();

			reg.des_engine().schedule_event(
					ptr_vm_suspend_evt_src_,
					cur_time,
					ptr_vm
				);

			ptr_vm->guest_system().application().simulation_model().request_tier_service_event_source(ptr_vm->guest_system().id()).connect(
					::dcs::functional::bind(
							&self_type::process_vm_request_service,
							this,
							::dcs::functional::placeholders::_1,
							::dcs::functional::placeholders::_2,
							ptr_vm
						)
				);
		}
		else
		{
			log_warn(DCS_EESIM_LOGGING_AT, "Cannot resume an non suspended-on virtual machine.");
		}

		DCS_DEBUG_TRACE("(" << this << ") END Do Resume virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");
	}


	private: void do_vm_migrate(virtual_machine_pointer const& ptr_vm, physical_machine_type& pm, bool pm_is_source)
	{
		DCS_DEBUG_ASSERT( ptr_vm );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Migrate virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");

		if (ptr_vm->power_state() == powered_on_power_status)
		{
			registry_type& reg(registry_type::instance());

			real_type cur_time(reg.des_engine().simulated_time());

			virtual_machine_migration_context<traits_type> evt_state;
			evt_state.vm_id = ptr_vm->id();
			evt_state.pm_id = pm.id();
			evt_state.pm_is_source = pm_is_source;

			if (pm_is_source)
			{
				vm_host_time_map_[ptr_vm->id()].push_back(::std::make_pair(cur_time, ::std::numeric_limits<real_type>::infinity()));

				ptr_vm->guest_system().application().simulation_model().request_tier_service_event_source(ptr_vm->guest_system().id()).connect(
						::dcs::functional::bind(
								&self_type::process_vm_request_service,
								this,
								::dcs::functional::placeholders::_1,
								::dcs::functional::placeholders::_2,
								ptr_vm
							)
					);
			}
			else
			{
				DCS_DEBUG_ASSERT( vm_host_time_map_.count(ptr_vm->id()) > 0 );
				DCS_DEBUG_ASSERT( vm_host_time_map_.at(ptr_vm->id()).size() > 0 );

				vm_host_time_map_[ptr_vm->id()].back().second = cur_time;

				update_utilization_profile(ptr_vm);
				update_share_profiles();

				ptr_vm->guest_system().application().simulation_model().request_tier_service_event_source(ptr_vm->guest_system().id()).disconnect(
						::dcs::functional::bind(
								&self_type::process_vm_request_service,
								this,
								::dcs::functional::placeholders::_1,
								::dcs::functional::placeholders::_2,
								ptr_vm
							)
					);
			}

			reg.des_engine().schedule_event(
					ptr_vm_migr_evt_src_,
					cur_time,
					evt_state
				);
		}
		else
		{
			log_warn(DCS_EESIM_LOGGING_AT, "Cannot migrate a non powered-on virtual machine.");
		}

		DCS_DEBUG_TRACE("(" << this << ") END Do Migrate virtual machine: " << *ptr_vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");
	}


	private: void do_vm_resource_share(virtual_machine_type const& vm, physical_resource_category category, real_type share)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(vm);
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(share);

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Change Resource Share of virtual machine: " << vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");

/*
		typedef typename physical_machine_type::vmm_type vmm_type;
		typedef typename vmm_type::virtual_machine_container vm_container;
		typedef typename vm_container::const_iterator vm_iterator;

		real_type aggr_share(0);

		vm_container vms(this->machine().vmm().virtual_machines(powered_on_power_status));
		vm_iterator vm_end_it(vms.end());
		for (vm_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_pointer ptr_vm(*vm_it);

			// paranoid-check: null
			DCS_DEBUG_ASSERT( ptr_vm );

			aggr_share += ptr_vm->resource_share(category);
		}

		real_type cur_time(registry_type::instance().des_engine().simulated_time());

		// paranoid-check: consistency
		DCS_DEBUG_ASSERT( cur_time >= share_stat_upd_time_ );

		// Safety check to prevent NaNs
		if (::dcs::math::float_traits<real_type>::definitely_greater(cur_time, share_stat_upd_time_) > 0)
		{
			(*ptr_share_stat_)(aggr_share, cur_time-share_stat_upd_time_);

		}
		share_stat_upd_time_ = cur_time;
*/

		update_share_profile(category);
//		update_resource_share_stat(category);

		DCS_DEBUG_TRACE("(" << this << ") END Do Change Resource Share of virtual machine: " << vm << " on physical machine: " << this->machine() << " (Clock: " << registry_type::instance().des_engine().simulated_time() << ")");

	}


	private: des_event_source_type& do_power_on_event_source()
	{
		return *ptr_pwron_evt_src_;
	}


	private: des_event_source_type const& do_power_on_event_source() const
	{
		return *ptr_pwron_evt_src_;
	}


	private: des_event_source_type& do_power_off_event_source()
	{
		return *ptr_pwroff_evt_src_;
	}


	private: des_event_source_type const& do_power_off_event_source() const
	{
		return *ptr_pwroff_evt_src_;
	}


	private: des_event_source_type& do_vm_power_on_event_source()
	{
		return *ptr_vm_pwron_evt_src_;
	}


	private: des_event_source_type const& do_vm_power_on_event_source() const
	{
		return *ptr_vm_pwron_evt_src_;
	}


	private: des_event_source_type& do_vm_power_off_event_source()
	{
		return *ptr_vm_pwroff_evt_src_;
	}


	private: des_event_source_type const& do_vm_power_off_event_source() const
	{
		return *ptr_vm_pwroff_evt_src_;
	}


	private: des_event_source_type& do_vm_suspend_event_source()
	{
		return *ptr_vm_suspend_evt_src_;
	}


	private: des_event_source_type const& do_vm_suspend_event_source() const
	{
		return *ptr_vm_suspend_evt_src_;
	}


	private: des_event_source_type& do_vm_resume_event_source()
	{
		return *ptr_vm_resume_evt_src_;
	}


	private: des_event_source_type const& do_vm_resume_event_source() const
	{
		return *ptr_vm_resume_evt_src_;
	}


	private: des_event_source_type& do_vm_migrate_event_source()
	{
		return *ptr_vm_migr_evt_src_;
	}


	private: des_event_source_type const& do_vm_migrate_event_source() const
	{
		return *ptr_vm_migr_evt_src_;
	}


	private: output_statistic_type const& do_consumed_energy() const
	{
		return *ptr_energy_stat_;
	}


	private: output_statistic_type const& do_uptime() const
	{
		return *ptr_uptime_stat_;
	}


	private: output_statistic_type const& do_utilization() const
	{
		return *ptr_util_stat_;
	}


	private: output_statistic_type const& do_share() const
	{
		return *ptr_share_stat_;
	}

	//@} Interface Member Functions


	//@{ Event Handlers

	private: void process_begin_of_sim(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing BEGIN-OF-SIMULATION (Clock: " << ctx.simulated_time() << ")");

		// Reset stats
		ptr_energy_stat_->reset();
		ptr_uptime_stat_->reset();
		ptr_util_stat_->reset();
		ptr_share_stat_->reset();

		last_share_upd_time_ = real_type(0);

		DCS_DEBUG_TRACE("(" << this << ") END Processing BEGIN-OF-SIMULATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");
//::std::cerr << "(" << this << ") BEGIN Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")" << ::std::endl;///XXX

		// Reset per-experiment stats
//		energy_ = uptime_
		uptime_ = last_pwron_time_
				= last_share_upd_time_
				= real_type/*zero*/();

		pwr_state_ = powered_off_power_status;

		res_profile_map_.clear();
		vm_host_time_map_.clear();

//::std::cerr << "(" << this << ") END Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")" << ::std::endl;///XXX
		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");
	}


	private: void process_sys_finit(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");
//::std::cerr << "[default_physical_machine_simulation_model] (" << this << ") BEGIN Processing SYSTEM-FINALIZATION -- PM: " << this->machine() << " (Clock: " << ctx.simulated_time() << ")" << ::std::endl;///XXX

		// Update experiment-level stats only if the machine is currently
		// powered-on (because, in the other case, stats should have already
		// been updated in the power-change event handler).
		if (this->power_state() == powered_on_power_status)
		{
			//update_experiment_stats(ctx);
			update_share_profiles();
			this->machine().power_off();
		}

		// Compute energy & utilization
#if 0
		typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;
		typedef typename virtual_machine_container::const_iterator vm_iterator;
		typedef user_request<traits_type> user_request_type;
		typedef ::std::vector<user_request_type> request_container;
		typedef typename request_container::const_iterator request_iterator;
		typedef typename utilization_profile_type::const_iterator profile_iterator;
		// - Update energy profile with utilizations due to not-yet-terminated requests running on powered-on VMs
		virtual_machine_container active_vms(this->machine().vmm().virtual_machines(powered_on_power_status));
		vm_iterator vm_end_it(active_vms.end());
		for (vm_iterator vm_it = active_vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_pointer ptr_vm(*vm_it);

			// check: valid VM pointer (paranoid)
			DCS_DEBUG_ASSERT( ptr_vm );

			request_container reqs(ptr_vm->guest_system().application().simulation_model().tier_in_service_requests(ptr_vm->guest_system().id()));
			request_iterator req_end_it(reqs.end());
			for (request_iterator req_it = reqs.begin(); req_it != req_end_it; ++req_it)
			{
				user_request_type const& req(*req_it);
				//FIXME: CPU resource category is hard-coded
				physical_resource_category category(cpu_resource_category);

				::std::vector<utilization_profile_type> profiles;
				profiles = req.tier_utilization_profiles(ptr_vm->guest_system().id(), category);
				if (profiles.size() > 0)
				{
					// The last profiles refers to the last visit (at this tier)
					utilization_profile_type const& profile(profiles.back());
					profile_iterator profile_end_it(profile.end());
					for (profile_iterator profile_it = profile.begin(); profile_it != profile_end_it; ++profile_it)
					{
						res_profile_map_[category](*profile_it);
					}
				}
			}
		}
#endif // 0
		// - Compute total energy as the sum of the energy consumed during each
		//   active time interval and machine utilization as the ratio between
		//   busy and up time.
		typedef user_request<traits_type> user_request_type;
		typedef ::std::vector<user_request_type> request_container;
		typedef typename request_container::const_iterator request_iterator;
		typedef typename utilization_profile_type::const_iterator profile_iterator;
		real_type idle_energy(this->machine().consumed_energy(0));
		real_type energy(0);
		real_type busy_time(0);
		if (res_profile_map_.size() > 0)
		{
			//FIXME: CPU resource category is hard-coded
			physical_resource_category category(cpu_resource_category);
			utilization_profile_type const& profile(res_profile_map_.at(category));
			profile_iterator profile_end_it(profile.end());
			for (profile_iterator profile_it = profile.begin(); profile_it != profile_end_it; ++profile_it)
			{
				typename utilization_profile_type::profile_item_type const& item(*profile_it);

				//FIXME: replace boost::icl::length with a wrapper function
				energy += (this->machine().consumed_energy(item.second)-idle_energy)*::boost::icl::length(item.first);
				busy_time += ::boost::icl::length(item.first);
			}
		}
		energy += idle_energy*uptime_;

//::std::cerr << "[default_physical_machine_simulation] PM: " << this->machine() << " - BUSY-TIME: " << busy_time << " - UPTIME: " << uptime_ << ::std::endl;//XXX
		// check: machine cannot be busy more than is up (paranoid check)
		DCS_DEBUG_ASSERT( ::dcs::math::float_traits<real_type>::definitely_less_equal(busy_time, uptime_) );

		// Update simulation-level stats
		(*ptr_energy_stat_)(energy);
		(*ptr_uptime_stat_)(uptime_);
		if (uptime_ > 0)
		{
			// The std::min function is used to compensate floating-point error which may
			// result in busy_time_ a bit greater than uptime_
			(*ptr_util_stat_)(::std::min(busy_time/uptime_, static_cast<real_type>(1)));
		}
		else
		{
			(*ptr_util_stat_)(0);
		}

//::std::cerr << "[default_physical_machine_simulation_model] (" << this << ") END Processing SYSTEM-FINALIZATION -- PM: " << this->machine() << " (Clock: " << ctx.simulated_time() << ")" << ::std::endl;///XXX
		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");
	}


//	private: void process_power_on(des_event_type const& evt, des_engine_context_type& ctx)
//	{
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
//
//		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing POWER-ON (Clock: " << ctx.simulated_time() << ")");
//
//		last_pwron_time_ = ctx.simulated_time();
//
//		DCS_DEBUG_TRACE("(" << this << ") END Processing POWER-ON (Clock: " << ctx.simulated_time() << ")");
//	}


//	private: void process_power_off(des_event_type const& evt, des_engine_context_type& ctx)
//	{
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
//
//		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing POWER-OFF (Clock: " << ctx.simulated_time() << ")");
//
//		// pre: make sure a power-off follows in time a power-on
//		DCS_DEBUG_ASSERT( ctx.simulated_time() >= last_pwron_time_ );
//
//		update_experiment_stats(ctx);
//
//		DCS_DEBUG_TRACE("(" << this << ") END Processing POWER-OFF (Clock: " << ctx.simulated_time() << ")");
//	}


	private: void process_vm_request_service(des_event_type const& evt, des_engine_context_type& ctx, virtual_machine_pointer const& ptr_vm)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ctx);

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing VM-REQUEST-SERVICE (Clock: " << ctx.simulated_time() << ")");

		// pre: virtual machine pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_vm );

		update_utilization_profile(
				ptr_vm,
				ptr_vm->guest_system().application().simulation_model().request_state(evt)
			);

		DCS_DEBUG_TRACE("(" << this << ") END Processing VM-REQUEST-SERVICE (Clock: " << ctx.simulated_time() << ")");
	}


	//@} Event Handlers


	private: power_status pwr_state_;
//	private: real_type energy_;
	private: real_type uptime_;
	private: real_type last_pwron_time_;
	private: des_event_source_pointer ptr_pwron_evt_src_;
	private: des_event_source_pointer ptr_pwroff_evt_src_;
	private: des_event_source_pointer ptr_vm_pwron_evt_src_;
	private: des_event_source_pointer ptr_vm_pwroff_evt_src_;
	private: des_event_source_pointer ptr_vm_suspend_evt_src_;
	private: des_event_source_pointer ptr_vm_resume_evt_src_;
	private: des_event_source_pointer ptr_vm_migr_evt_src_;
	private: output_statistic_pointer ptr_energy_stat_;
	private: output_statistic_pointer ptr_uptime_stat_;
	private: output_statistic_pointer ptr_util_stat_;
	private: output_statistic_pointer ptr_share_stat_;
	private: real_type last_share_upd_time_;
	private: ::std::map<physical_resource_category,utilization_profile_type> res_profile_map_;
	private: virtual_machine_hosting_time_map vm_host_time_map_;
//	private: virtual_machine_share_time_map vm_share_time_map_;
	private: bool can_pm_auto_pow_off_;
}; // default_physical_machine_simulation_model: public base_physical_machine_simulation_model<TraitsT>

template <typename TraitsT>
const ::std::string default_physical_machine_simulation_model<TraitsT>::poweron_event_source_name("Physical Machine Power-on");

template <typename TraitsT>
const ::std::string default_physical_machine_simulation_model<TraitsT>::poweroff_event_source_name("Physical Machine Power-off");

template <typename TraitsT>
const ::std::string default_physical_machine_simulation_model<TraitsT>::vm_poweron_event_source_name("Virtual Machine Power-on");

template <typename TraitsT>
const ::std::string default_physical_machine_simulation_model<TraitsT>::vm_poweroff_event_source_name("Virtual Machine Power-off");

template <typename TraitsT>
const ::std::string default_physical_machine_simulation_model<TraitsT>::vm_suspend_event_source_name("Virtual Machine Suspend");

template <typename TraitsT>
const ::std::string default_physical_machine_simulation_model<TraitsT>::vm_resume_event_source_name("Virtual Machine Resume");

template <typename TraitsT>
const ::std::string default_physical_machine_simulation_model<TraitsT>::vm_migration_event_source_name("Virtual Machine Migration");

}} // Namespace dcs::eesim

#endif // DCS_EESIM_DEFAULT_PHYSICAL_MACHINE_SIMULATION_MODEL_HPP
