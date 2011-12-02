/**
 * \file src/dcs/eesim/best_fit_decreasing_migration_controller.hpp
 *
 * \brief Migration Controller based on Mixed-Integer Nonlinear Programming.
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

#ifndef DCS_EESIM_BEST_FIT_DECREASING_MIGRATION_CONTROLLER_HPP
#define DCS_EESIM_BEST_FIT_DECREASING_MIGRATION_CONTROLLER_HPP


#include <algorithm>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/des/statistic_categories.hpp>
#include <dcs/eesim/base_migration_controller.hpp>
#include <dcs/eesim/detail/placement_strategy_utility.hpp>
#include <dcs/eesim/logging.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/power_status.hpp>
#include <dcs/exception.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class best_fit_decreasing_migration_controller: public base_migration_controller<TraitsT>
{
	private: typedef base_migration_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::data_center_pointer data_center_pointer;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef typename base_type::statistic_type statistic_type;
	private: typedef ::dcs::des::mean_estimator<real_type,uint_type> statistic_impl_type;
	private: typedef ::dcs::shared_ptr<statistic_type> statistic_pointer;
	private: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
	private: typedef ::std::map<physical_resource_category,real_type> resource_utilization_map;
	private: typedef ::std::map<virtual_machine_identifier_type,resource_utilization_map> virtual_machine_utilization_map;


	private: static const ::dcs::des::statistic_category utilization_statistic_category = ::dcs::des::mean_statistic;
	public: static const real_type default_ewma_smoothing_factor;


	public: best_fit_decreasing_migration_controller()
	: base_type(),
	  ewma_smooth_(default_ewma_smoothing_factor),
	  count_(0),
	  fail_count_(0),
	  migr_count_(0),
	  migr_rate_num_(0),
	  migr_rate_den_(0),
	  ptr_cost_(new statistic_impl_type()),
	  ptr_num_migr_(new statistic_impl_type()),
	  ptr_migr_rate_(new statistic_impl_type())
	{
//		init();
	}


	public: best_fit_decreasing_migration_controller(data_center_pointer const& ptr_dc,
													 real_type ts,
													 real_type smooth_factor = default_ewma_smoothing_factor)
	: base_type(ptr_dc, ts),
	  ewma_smooth_(smooth_factor),
	  count_(0),
	  fail_count_(0),
	  migr_count_(0),
	  migr_rate_num_(0),
	  migr_rate_den_(0),
	  ptr_cost_(new statistic_impl_type()),
	  ptr_num_migr_(new statistic_impl_type()),
	  ptr_migr_rate_(new statistic_impl_type())
	{
//		init();
	}


	/// Copy constructor.
	private: best_fit_decreasing_migration_controller(best_fit_decreasing_migration_controller const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: best_fit_decreasing_migration_controller& operator=(best_fit_decreasing_migration_controller const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
	}


//	public: ~best_fit_decreasing_migration_controller()
//	{
//	}


	//@{ Interface Member Functions

//	protected: void do_controlled_data_center(data_center_pointer const& ptr_data_center)
//	{
//	}


	protected: void do_process_begin_of_sim(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process BEGIN-OF-SIMULATION event (Clock: " << ctx.simulated_time() << ")");

		ptr_cost_->reset();
		ptr_num_migr_->reset();
		ptr_migr_rate_->reset();

		DCS_DEBUG_TRACE("(" << this << ") END Do Process BEGIN-OF-SIMULATION event (Clock: " << ctx.simulated_time() << ")");
	}


	protected: void do_process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process SYSTEM-INITIALIZATION event (Clock: " << ctx.simulated_time() << ")");

		count_ = fail_count_
			   = migr_count_
			   = migr_rate_num_
			   = migr_rate_den_
			   = uint_type(0);
		vm_util_map_.clear();

		DCS_DEBUG_TRACE("(" << this << ") END Do Process SYSTEM-INITIALIZATION event (Clock: " << ctx.simulated_time() << ")");
	}


	protected: void do_process_sys_finit(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process SYSTEM-FINALIZATION event (Clock: " << ctx.simulated_time() << ")");

		(*ptr_num_migr_)(migr_count_);
        // For migration rate, use the weighted harmonic mean (see the "Workload Book" by Feitelson)
		(*ptr_migr_rate_)(static_cast<real_type>(migr_rate_num_)/static_cast<real_type>(migr_rate_den_));

		DCS_DEBUG_TRACE("(" << this << ") END Do Process SYSTEM-FINALIZATION event (Clock: " << ctx.simulated_time() << ")");
	}


	private: void do_process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Process CONTROL event (Clock: " << ctx.simulated_time() << " - Count: " << count_ << " - Fail-Count: " << fail_count_ << ")");

		typedef typename base_type::data_center_type data_center_type;
		typedef typename data_center_type::application_type application_type; 
		typedef typename data_center_type::physical_machine_type pm_type;
		typedef typename data_center_type::physical_machine_pointer pm_pointer;
		typedef typename data_center_type::virtual_machine_type vm_type;
		typedef typename data_center_type::virtual_machine_pointer vm_pointer;
		typedef typename pm_type::identifier_type pm_identifier_type;
		typedef typename vm_type::identifier_type vm_identifier_type;
		typedef typename application_type::application_tier_type application_tier_type;
		typedef ::std::vector<pm_pointer> pm_container;
		typedef ::std::vector<vm_pointer> vm_container;
		typedef typename pm_container::const_iterator pm_iterator;
		typedef typename vm_container::const_iterator vm_iterator;
//		typedef ::std::pair<physical_resource_category,real_type> share_type;
//		typedef ::std::vector<share_type> share_container;
		typedef typename application_tier_type::resource_share_type share_type;
#ifdef DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
		typedef typename base_type::vm_share_observer::share_container share_container;
#else // DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
		typedef typename application_tier_type::resource_share_container share_container;
#endif // DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
		typedef typename share_container::const_iterator share_iterator;
		typedef ::std::map<physical_resource_category,real_type> resource_share_map;
		typedef ::std::map<physical_resource_category,real_type> resource_utilization_map;
		typedef typename base_type::virtual_machines_placement_type virtual_machines_placement_type;
		typedef typename application_type::reference_physical_resource_type ref_resource_type;
		typedef typename application_type::reference_physical_resource_container ref_resource_container;
		typedef typename ref_resource_container::const_iterator ref_resource_iterator;

		++count_;

		data_center_type& dc(this->controlled_data_center());

		pm_container sorted_pms;
		pm_container pms;
		pms = dc.physical_machines(powered_on_power_status);
		if (!pms.empty())
		{
			::std::sort(pms.begin(),
						pms.end(),
						detail::ptr_physical_machine_greater_comparator<pm_type>());
			sorted_pms.insert(sorted_pms.end(), pms.begin(), pms.end());
		}
		pms = dc.physical_machines(suspended_power_status);
		if (!pms.empty())
		{
			::std::sort(pms.begin(),
						pms.end(),
						detail::ptr_physical_machine_greater_comparator<pm_type>());
			sorted_pms.insert(sorted_pms.end(), pms.begin(), pms.end());
		}
		pms = dc.physical_machines(powered_off_power_status);
		if (!pms.empty())
		{
			::std::sort(pms.begin(),
						pms.end(),
						detail::ptr_physical_machine_greater_comparator<pm_type>());
			sorted_pms.insert(sorted_pms.end(), pms.begin(), pms.end());
		}
		pms.clear();

		// Sort virtual machines according to their shares
        vm_container sorted_vms(dc.active_virtual_machines());
#ifdef DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
		::std::map<typename traits_type::virtual_machine_identifier_type, share_container> wanted_shares;
		typedef typename base_type::vm_observer_container::const_iterator vm_observer_iterator;
		vm_observer_iterator vm_obs_end_it(this->vm_observer_map().end());
		for (vm_observer_iterator it = this->vm_observer_map().begin(); it != vm_obs_end_it; ++it)
		{
			wanted_shares[it->first] = share_container(it->second->wanted_shares.begin(), it->second->wanted_shares.end());
		}
		::std::sort(sorted_vms.begin(),
					sorted_vms.end(),
					detail::ptr_virtual_machine_greater_by_share_comparator<vm_type>(wanted_shares.begin(), wanted_shares.end()));
		wanted_shares.clear();
#else // DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
        ::std::sort(sorted_vms.begin(),
                    sorted_vms.end(),
                    detail::ptr_virtual_machine_greater_comparator<vm_type>());
#endif // DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS

		uint_type num_vms(0);

		virtual_machines_placement_type deployment;

		vm_iterator vm_end_it(sorted_vms.end());

//		// Update VMs utilization stats
//		for (vm_iterator vm_it = sorted_vms.begin(); vm_it != vm_end_it; ++vm_it)
//		{
//			vm_pointer ptr_vm(*vm_it);
//
//			// check: paranoid check
//			DCS_DEBUG_ASSERT( ptr_vm );
//
//			if (ptr_vm->power_state() != powered_on_power_status)
//			{
//				// Non active VMs should not occupy resources
//				continue;
//			}
//
//			++num_vms;
//
//			real_type new_value(0);
//
//			// Retrieve current resource utilization
//			new_value =  ptr_vm->guest_system().application().simulation_model().actual_tier_utilization(
//							ptr_vm->guest_system().id()
//				);
//			// Scale in terms of reference machine
//			new_value = scale_resource_utilization(
//							ptr_vm->vmm().hosting_machine().resource(cpu_resource_category)->capacity(),
//							ptr_vm->guest_system().application().reference_resource(cpu_resource_category).capacity(),
//							new_value,
//							ptr_vm->guest_system().application().reference_resource(cpu_resource_category).utilization_threshold()
//				);
//
//			if (count_ > 1)
//			{
//				vm_util_map_[ptr_vm->id()] = ewma_smooth_*new_value + (1-ewma_smooth_)*vm_util_map_.at(ptr_vm->id());
//			}
//			else
//			{
//				vm_util_map_[ptr_vm->id()] = new_value;
//			}
//		}

		// Apply best-fit-decreasing strategy

		for (vm_iterator vm_it = sorted_vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			vm_pointer ptr_vm(*vm_it);

			// paranoid-check: valid pointer.
			DCS_DEBUG_ASSERT( ptr_vm );

			if (ptr_vm->power_state() != powered_on_power_status)
			{
//				// Non active VMs should not occupy resources
//				dc.displace_virtual_machine(ptr_vm, false);
				continue;
			}

			++num_vms;

			// Compute and update VM utilization map

			ref_resource_container rress(ptr_vm->guest_system().application().reference_resources());
			ref_resource_iterator rress_end_it(rress.end());
			for (ref_resource_iterator rress_it = rress.begin(); rress_it != rress_end_it; ++rress_it)
			{
				ref_resource_type res(*rress_it);

				//TODO: CPU resource category not yet handle in app simulation model
				DCS_ASSERT(
						res.category() == cpu_resource_category,
						DCS_EXCEPTION_THROW(
							::std::runtime_error,
							"Resource categories other than CPU are not yet implemented."
						)
					);

				real_type new_util(0);

				// Retrieve current resource utilization
				new_util =  ptr_vm->guest_system().application().simulation_model().actual_tier_utilization(
								ptr_vm->guest_system().id()
					);
				// Scale in terms of reference machine
				new_util = scale_resource_utilization(
								ptr_vm->vmm().hosting_machine().resource(res.category())->capacity(),
								res.capacity(),
								new_util,
								res.utilization_threshold()
					);

				if (count_ > 1)
				{
					vm_util_map_[ptr_vm->id()][res.category()] = ewma_smooth_*new_util + (1-ewma_smooth_)*vm_util_map_.at(ptr_vm->id()).at(res.category());
				}
				else
				{
					vm_util_map_[ptr_vm->id()][res.category()] = new_util;
				}
			}

			application_type const& app(ptr_vm->guest_system().application());

			// Retrieve the share for every resource of the VM guest system
#ifdef DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
			// As reference shares uses the wanted shares collected by
			// monitoring each VM.
			share_container ref_shares(this->vm_observer_map().at(ptr_vm->id())->wanted_shares);
#else // DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
			// As reference shares uses the ones defined by the tier
			// specifications.
			share_container ref_shares(ptr_vm->guest_system().resource_shares());
#endif // DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS

			// For each physical machine PM, try to deploy current VM on PM
			// until a suitable machine (i.e., a machine with sufficient free
			// capacity) is found.
			bool placed(false);
			pm_iterator pm_end_it(sorted_pms.end());
			share_iterator ref_share_end_it(ref_shares.end());
			for (pm_iterator pm_it = sorted_pms.begin(); pm_it != pm_end_it && !placed; ++pm_it)
			{
				pm_pointer ptr_pm(*pm_it);

				// paranoid-check: valid pointer.
				DCS_DEBUG_ASSERT( ptr_pm );

				// Reference to actual resource shares
				resource_share_map shares;

				for (share_iterator ref_share_it = ref_shares.begin(); ref_share_it != ref_share_end_it; ++ref_share_it)
				{
					physical_resource_category ref_category(ref_share_it->first);
					real_type ref_share(ref_share_it->second);

					real_type ref_capacity(app.reference_resource(ref_category).capacity());
					//real_type ref_threshold(app.reference_resource(ref_category).utilization_threshold());

					real_type actual_capacity(ptr_pm->resource(ref_category)->capacity());
					//real_type actual_threshold(ptr_pm->resource(ref_category)->utilization_threshold());

					real_type share;
					share = scale_resource_share(ref_capacity,
												 //ref_threshold,
												 actual_capacity,
												 //actual_threshold,
												 ref_share);

					shares[ref_category] = share;
				}

				// Reference to actual resource utilizaition
				resource_utilization_map utils;
				for (ref_resource_iterator rress_it = rress.begin(); rress_it != rress_end_it; ++rress_it)
				{
					ref_resource_type res(*rress_it);

					real_type util(vm_util_map_.at(ptr_vm->id()).at(res.category()));

					util = scale_resource_utilization(res.capacity(),
													  ptr_pm->resource(res.category())->capacity(),
													  util,
													  ptr_pm->resource(res.category())->utilization_threshold());
					if (shares.count(res.category()))
					{
						util /= shares.at(res.category());
					}

					utils[res.category()] = util;
				}

				// Try to place current VM on current PM
				placed = deployment.try_place(*ptr_vm,
											  *ptr_pm,
											  shares.begin(),
											  shares.end(),
											  utils.begin(),
											  utils.end(),
											  dc);
::std::cerr << "[bfd_migration_controller] Evaluating VM: " << *ptr_vm << " - PM: " << *ptr_pm << " - SHARE: " << shares.at(cpu_resource_category) << " - UTIL: " << utils.at(cpu_resource_category) << " ==> " << std::boolalpha << placed << ::std::endl;//XXX
			}

			if (!placed)
			{
				++fail_count_;

				::std::ostringstream oss;
				oss << "Failed to find a placement for VM: " << *ptr_vm << ". Skip migration.";
				log_warn(DCS_EESIM_LOGGING_AT, oss.str());

				return;
			}
		}

		if (num_vms > 0)
		{
			uint_type num_migrs(0);
			num_migrs = this->migrate(deployment);

			migr_count_ += num_migrs;
			// For migration rate, use the weighted harmonic mean (see the "Workload Book" by Feitelson)
			migr_rate_den_ += num_vms;
			migr_rate_num_ += num_migrs;
		}
		else
		{
			log_warn(DCS_EESIM_LOGGING_AT, "No VM is running. Power-off active PMs.");

			pm_container pms(dc.physical_machines(powered_on_power_status));
			pm_iterator pm_end_it(pms.end());
			for (pm_iterator pm_it = pms.begin(); pm_it != pm_end_it; ++pm_it)
			{
				pm_pointer ptr_pm(*pm_it);

				// paranoid-check: null
				DCS_DEBUG_ASSERT( ptr_pm );

				ptr_pm->power_off();
			}
		}

/*
		// Check solution and act accordingly

		if (ptr_solver_->result().solved())
		{
			if (ptr_solver_->result().cost() < last_cost_)
			{
				(*ptr_cost_)(ptr_solver_->result().cost());

				physical_machine_id_map inactive_pms;

				// Populate the inactive PMs container with all powered-on machines
				{
					physical_machine_container active_pms(dc.physical_machines(powered_on_power_status));
					physical_machine_iterator pm_end_it(active_pms.end());
					for (physical_machine_iterator pm_it = active_pms.begin(); pm_it != pm_end_it; ++pm_it)
					{
						physical_machine_pointer ptr_pm(*pm_it);

						inactive_pms[ptr_pm->id()] = ptr_pm;
					}
				}

				// Migrate VMs
				{
	//				dc.displace_active_virtual_machines(false);

					physical_virtual_machine_map pm_vm_map(ptr_solver_->result().placement());
					physical_virtual_machine_iterator pm_vm_end_it(pm_vm_map.end());
					for (physical_virtual_machine_iterator pm_vm_it = pm_vm_map.begin(); pm_vm_it != pm_vm_end_it; ++pm_vm_it)
					{
						physical_machine_pointer ptr_pm(dc.physical_machine_ptr(pm_vm_it->first.first));
						virtual_machine_pointer ptr_vm(dc.virtual_machine_ptr(pm_vm_it->first.second));
						resource_share_container shares(pm_vm_it->second);

						// check: paranoid check
						DCS_DEBUG_ASSERT( ptr_pm );
						// check: paranoid check
						DCS_DEBUG_ASSERT( ptr_vm );

	//::std::cerr << "Going to migrate VM (" << pm_vm_it->first.second << "): " << *ptr_vm << " into PM (" << pm_vm_it->first.first << "): " << *ptr_pm << ::std::endl; //XXX
						if (ptr_vm->vmm().hosting_machine().id() != ptr_pm->id())
						{
							++migr_count_;
						}

						dc.migrate_virtual_machine(ptr_vm, ptr_pm, shares.begin(), shares.end());

						if (inactive_pms.count(ptr_pm->id()) > 0)
						{
							inactive_pms.erase(ptr_pm->id());
						}
					}
				}

				// Turn off unused PMs
				{
					physical_machine_id_iterator pm_id_end_it(inactive_pms.end());
					for (physical_machine_id_iterator pm_id_it = inactive_pms.begin(); pm_id_it != pm_id_end_it; ++pm_id_it)
					{
						physical_machine_pointer ptr_pm(pm_id_it->second);

						ptr_pm->power_off();
					}
				}
			}
			else
			{
				log_warn(DCS_EESIM_LOGGING_AT, "New optimal solution is worst than the previous one. Skip migration.");
				++fail_count_;
			}
		}
		else
		{
			log_warn(DCS_EESIM_LOGGING_AT, "Failed to solve optimization problem. Skip migration.");
			++fail_count_;
		}
*/

		DCS_DEBUG_TRACE("(" << this << ") END Do Process CONTROL event (Clock: " << ctx.simulated_time() << " - Count: " << count_ << " - Fail-Count: " << fail_count_ << ")");
	}


	private: statistic_type const& do_num_migrations() const
	{
		return *ptr_num_migr_;
	}


	private: statistic_type const& do_migration_rate() const
	{
		return *ptr_migr_rate_;
	}

	//@} Interface Member Functions



	private: real_type ewma_smooth_;
	private: uint_type count_;
	private: uint_type fail_count_;
	private: uint_type migr_count_;
	private: uint_type migr_rate_num_;
	private: uint_type migr_rate_den_;
	private: statistic_pointer ptr_cost_;
	private: statistic_pointer ptr_num_migr_;
	private: statistic_pointer ptr_migr_rate_;
	private: virtual_machine_utilization_map vm_util_map_;
}; // best_fit_decreasing_migration_controller

template <typename TraitsT>
const typename best_fit_decreasing_migration_controller<TraitsT>::real_type best_fit_decreasing_migration_controller<TraitsT>::default_ewma_smoothing_factor(0.70);

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BEST_FIT_DECREASING_MIGRATION_CONTROLLER_HPP
