/**
 * \file dcs/des/cloud/optimal_migration_controller.hpp
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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_CLOUD_OPTIMAL_MIGRATION_CONTROLLER_HPP
#define DCS_DES_CLOUD_OPTIMAL_MIGRATION_CONTROLLER_HPP


#include <ctime>//XXX
#include <dcs/debug.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/des/statistic_categories.hpp>
#include <dcs/des/cloud/base_migration_controller.hpp>
//#include <dcs/des/cloud/detail/ampl/vm_placement_minlp_solver.hpp>
//#include <dcs/des/cloud/detail/couenne/vm_placement_minlp_solver.hpp>
//#include <dcs/des/cloud/detail/neos/vm_placement_minlp_solver.hpp>
#include <dcs/des/cloud/detail/vm_placement_optimal_solvers.hpp>
#include <dcs/des/cloud/logging.hpp>
#include <dcs/des/cloud/physical_resource_category.hpp>
#include <dcs/des/cloud/power_status.hpp>
#include <dcs/exception.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <limits>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>


namespace dcs { namespace des { namespace cloud {

template <typename TraitsT>
class optimal_migration_controller: public base_migration_controller<TraitsT>
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
	private: typedef typename base_type::data_center_type data_center_type;
	private: typedef typename data_center_type::virtual_machines_placement_type virtual_machines_placement_type;
	private: typedef typename data_center_type::physical_machine_type physical_machine_type;
	private: typedef typename data_center_type::physical_machine_pointer physical_machine_pointer;
	private: typedef typename data_center_type::virtual_machine_type virtual_machine_type;
	private: typedef typename data_center_type::virtual_machine_pointer virtual_machine_pointer;
	private: typedef typename virtual_machine_type::application_tier_type application_tier_type;
	//private: typedef detail::migration_controller::ampl_minlp_solver_impl<traits_type> optimal_solver_type;
	//private: typedef detail::ampl::vm_placement_minlp_solver<traits_type> optimal_solver_type;
	//private: typedef detail::neos::vm_placement_minlp_solver<traits_type> optimal_solver_type;
	public: typedef base_optimal_solver_params<traits_type> optimal_solver_params_type;
	private: typedef detail::base_vm_placement_optimal_solver<traits_type> optimal_solver_type;
	private: typedef ::dcs::shared_ptr<optimal_solver_type> optimal_solver_pointer;
	private: typedef typename base_type::statistic_type statistic_type;
	private: typedef ::dcs::des::mean_estimator<real_type,uint_type> statistic_impl_type;
	private: typedef ::dcs::shared_ptr<statistic_type> statistic_pointer;
	private: typedef typename traits_type::physical_machine_identifier_type physical_machine_identifier_type;
	private: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
	private: typedef ::std::map<virtual_machine_identifier_type,real_type> virtual_machine_utilization_map;


	private: static const ::dcs::des::statistic_category utilization_statistic_category = ::dcs::des::mean_statistic;
	public: static const real_type default_power_cost_weight;
	public: static const real_type default_migration_cost_weight;
	public: static const real_type default_sla_cost_weight;
	public: static const real_type default_ewma_smoothing_factor;


	public: optimal_migration_controller()
	: base_type(),
	  wp_(default_power_cost_weight),
	  wm_(default_migration_cost_weight),
	  ws_(default_sla_cost_weight),
	  ewma_smooth_(default_ewma_smoothing_factor),
	  count_(0),
	  fail_count_(0),
	  migr_count_(0),
	  migr_rate_num_(0),
	  migr_rate_den_(0),
	  ptr_cost_(new statistic_impl_type()),
	  ptr_num_migr_(new statistic_impl_type()),
	  ptr_migr_rate_(new statistic_impl_type()),
	  ptr_solver_()
	{
//		init();
	}


	public: optimal_migration_controller(data_center_pointer const& ptr_dc,
										 real_type ts,
										 optimal_solver_params_type const& solver_params,
										 real_type wp = default_power_cost_weight,
										 real_type wm = default_migration_cost_weight,
										 real_type ws = default_sla_cost_weight,
										 real_type smooth_factor = default_ewma_smoothing_factor)
	: base_type(ptr_dc, ts),
	  wp_(wp),
	  wm_(wm),
	  ws_(ws),
	  ewma_smooth_(smooth_factor),
	  count_(0),
	  fail_count_(0),
	  migr_count_(0),
	  migr_rate_num_(0),
	  migr_rate_den_(0),
	  ptr_cost_(new statistic_impl_type()),
	  ptr_num_migr_(new statistic_impl_type()),
	  ptr_migr_rate_(new statistic_impl_type()),
	  ptr_solver_(detail::make_vm_placement_optimal_solver(solver_params))
	{
//		init();
	}


	/// Copy constructor.
	private: optimal_migration_controller(optimal_migration_controller const& that)
	{
		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: optimal_migration_controller& operator=(optimal_migration_controller const& rhs)
	{
		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
	}


//	public: ~optimal_migration_controller()
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

{//XXX
::std::time_t t(::std::time(0));//XXX
::std::string st(::std::asctime(::std::localtime(&t)));
::std::cerr << "[optimal_migration_controller] BEGIN Do Process CONTROL event (Clock: " << ctx.simulated_time() << " - Count: " << count_ << " - Fail-Count: " << fail_count_ << " - #Migrations: " << migr_count_ << " - Real-Clock: " << st.substr(0, st.size()-1) <<  " (" << static_cast< unsigned long >(t) << " secs since the Epoch" << "))" << ::std::endl;//XXX
}//XXX
		typedef typename optimal_solver_type::problem_result_type optimal_solver_result_type;
		typedef typename application_tier_type::resource_share_container ref_share_container;
		//typedef typename optimal_solver_type::resource_share_container share_container;
#ifdef DCS_DES_CLOUD_EXP_MIGR_CONTROLLER_MONITOR_VMS
		typedef typename base_type::vm_share_observer::share_container share_container;
#else // DCS_DES_CLOUD_EXP_MIGR_CONTROLLER_MONITOR_VMS
		typedef ::std::map<physical_resource_category,real_type> share_container;
#endif // DCS_DES_CLOUD_EXP_MIGR_CONTROLLER_MONITOR_VMS
		typedef ::std::vector<physical_machine_pointer> physical_machine_container;
		typedef typename physical_machine_container::const_iterator physical_machine_iterator;
		typedef ::std::map<physical_machine_identifier_type,physical_machine_pointer> physical_machine_id_map;
		typedef typename physical_machine_id_map::iterator physical_machine_id_iterator;
		typedef typename base_type::virtual_machines_placement_type virtual_machines_placement_type;
		typedef typename data_center_type::application_type application_type;
        typedef typename application_type::reference_physical_resource_type ref_resource_type;
        typedef typename application_type::reference_physical_resource_container ref_resource_container;
        typedef typename ref_resource_container::const_iterator ref_resource_iterator;
		typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;
		typedef typename virtual_machine_container::const_iterator virtual_machine_iterator;
//		typedef ::std::vector<statistic_pointer> statistic_container;
//		typedef typename statistic_container::const_iterator statistic_iterator;


		++count_;

		data_center_type& dc(this->controlled_data_center());
		uint_type num_vms(0);
		::std::map<typename traits_type::virtual_machine_identifier_type, share_container> wanted_share_map;

		// Update VMs utilization stats
		virtual_machine_container vms(dc.active_virtual_machines());
		virtual_machine_iterator vm_end_it(vms.end());
		for (virtual_machine_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_pointer ptr_vm(*vm_it);

			// paranoid-check: null
			DCS_DEBUG_ASSERT( ptr_vm );

			if (ptr_vm->power_state() != powered_on_power_status)
			{
//				// Non active VMs should not occupy resources
//				dc.displace_virtual_machine(ptr_vm, false);
				continue;
			}

			++num_vms;

			// Retrieve reference resource shares
			share_container ref_shares;
			{
				ref_share_container tmp_shares(ptr_vm->guest_system().resource_shares());
				ref_shares = share_container(tmp_shares.begin(), tmp_shares.end());
			}

#ifdef DCS_DES_CLOUD_EXP_MIGR_CONTROLLER_MONITOR_VMS
			// As observed shares uses the wanted shares collected by
			// monitoring each VM.

			wanted_share_map[ptr_vm->id()] = share_container(this->vm_observer_map().at(ptr_vm->id())->wanted_shares.begin(),
															 this->vm_observer_map().at(ptr_vm->id())->wanted_shares.end());
#else // DCS_DES_CLOUD_EXP_MIGR_CONTROLLER_MONITOR_VMS
			// As reference shares uses the ones defined by the tier
			// specifications.

			wanted_share_map[ptr_vm->id()] = share_container(ref_shares.begin(),
															 ref_shares.end());
#endif // DCS_DES_CLOUD_EXP_MIGR_CONTROLLER_MONITOR_VMS

			// Compute and update VM utilization map

			ref_resource_container rress(ptr_vm->guest_system().application().reference_resources());
			ref_resource_iterator rress_end_it(rress.end());
			for (ref_resource_iterator rress_it = rress.begin(); rress_it != rress_end_it; ++rress_it)
			{
				ref_resource_type res(*rress_it);

				//TODO: CPU resource category not yet handle in app simulation model and optimization problem
				DCS_ASSERT(
						res.category() == cpu_resource_category,
						DCS_EXCEPTION_THROW(
							::std::runtime_error,
							"Resource categories other than CPU are not yet implemented."
						)
					);

				real_type obs_util(0);

				// Retrieve current resource utilization
				obs_util =  ptr_vm->guest_system().application().simulation_model().actual_tier_utilization(
								ptr_vm->guest_system().id()
					);
::std::cerr << "[optimal_migration_controller] Observed Utilization " << obs_util << ::std::endl;//XXX

				real_type obs_share(wanted_share_map.at(ptr_vm->id()).at(res.category()));
::std::cerr << "[optimal_migration_controller] Observed Share " << obs_share << ::std::endl;//XXX

				// Scale share in terms of actual machine
                obs_share = scale_resource_share(res.capacity(),
												 ptr_vm->vmm().hosting_machine().resource(res.category())->capacity(),
												 obs_share);

::std::cerr << "[optimal_migration_controller] Scaled (ref -> actual) Observed Share " << obs_share << ::std::endl;//XXX
::std::cerr << "[optimal_migration_controller] Reference Share " << ref_shares.at(res.category()) << ::std::endl;//XXX

				// Scale utilization in terms of reference machine
				obs_util = scale_resource_utilization(ptr_vm->vmm().hosting_machine().resource(res.category())->capacity(),
													  obs_share,
													  res.capacity(),
													  ref_shares.at(res.category()),
													  obs_util,
													  res.utilization_threshold());
::std::cerr << "[optimal_migration_controller] Scaled (actual -> reference) Observed Utilization " << obs_util << ::std::endl;//XXX

				if (count_ > 1)
				{
					//vm_util_map_[ptr_vm->id()][res.category()] = ewma_smooth_*obs_util + (1-ewma_smooth_)*vm_util_map_.at(ptr_vm->id());
					vm_util_map_[ptr_vm->id()] = ewma_smooth_*obs_util + (1-ewma_smooth_)*vm_util_map_.at(ptr_vm->id());
				}
				else
				{
					//vm_util_map_[ptr_vm->id()][res.category()] = obs_util;
					vm_util_map_[ptr_vm->id()] = obs_util;
				}
::std::cerr << "[optimal_migration_controller] Filtered Observed Utilization " << vm_util_map_.at(ptr_vm->id())/*.at(res.category())*/ << ::std::endl;//XX
			}
		}
		vms.clear();

		if (num_vms > 0)
		{
			// Solve the optimization problem

			//optimal_solver_type solver;

//			::std::map<typename traits_type::virtual_machine_identifier_type, share_container> wanted_share_map;
//#ifdef DCS_DES_CLOUD_EXP_MIGR_CONTROLLER_MONITOR_VMS
//			// As reference shares uses the wanted shares collected by
//			// monitoring each VM.
//
//			typedef typename base_type::vm_observer_container::const_iterator vm_observer_iterator;
//			vm_observer_iterator vm_obs_end_it(this->vm_observer_map().end());
//			for (vm_observer_iterator it = this->vm_observer_map().begin(); it != vm_obs_end_it; ++it)
//			{
//				wanted_share_map[it->first] = resource_share_container(it->second->wanted_shares.begin(), it->second->wanted_shares.end());
//			}
//#else // DCS_DES_CLOUD_EXP_MIGR_CONTROLLER_MONITOR_VMS
//			// As reference shares uses the ones defined by the tier
//			// specifications.
//
//			vm_container vms(dc.active_virtual_machines());
//			typedef typename vm_container::const_iterator vm_iterator;
//			vm_iterator vm_end_it(vms.end());
//			for (vm_iterator it = vms.begin(); it != vm_end_it; ++it)
//			{
//				vm_pointer ptr_vm(*it);
//
//				// paranoid-check: null
//				DCS_DEBUG_ASSERT( ptr_vm );
//
//				wanted_share_map[ptr_vm->id()] = ptr_vm->guest_system().resource_shares();
//			}
//			vms.clear();
//#endif // DCS_DES_CLOUD_EXP_MIGR_CONTROLLER_MONITOR_VMS

			ptr_solver_->solve(dc, wp_, wm_, ws_, vm_util_map_.begin(), vm_util_map_.end(), wanted_share_map.begin(), wanted_share_map.end());

			// Check solution and act accordingly

			if (ptr_solver_->result().solved())
			{
				typedef typename optimal_solver_result_type::resource_share_container opt_share_container;
				typedef typename optimal_solver_result_type::physical_virtual_machine_map opt_physical_virtual_machine_map;
				typedef typename opt_physical_virtual_machine_map::const_iterator opt_physical_virtual_machine_iterator;

				(*ptr_cost_)(ptr_solver_->result().cost());

				virtual_machines_placement_type deployment;

				opt_physical_virtual_machine_map pm_vm_map(ptr_solver_->result().placement());
				opt_physical_virtual_machine_iterator pm_vm_end_it(pm_vm_map.end());
				for (opt_physical_virtual_machine_iterator pm_vm_it = pm_vm_map.begin(); pm_vm_it != pm_vm_end_it; ++pm_vm_it)
				{
					physical_machine_pointer ptr_pm(dc.physical_machine_ptr(pm_vm_it->first.first));
					virtual_machine_pointer ptr_vm(dc.virtual_machine_ptr(pm_vm_it->first.second));
					opt_share_container const& shares(pm_vm_it->second);

					// check: paranoid check
					DCS_DEBUG_ASSERT( ptr_pm );
					// check: paranoid check
					DCS_DEBUG_ASSERT( ptr_vm );

					deployment.place(*ptr_vm,
									 *ptr_pm,
									 shares.begin(),
									 shares.end());
				}

				uint_type num_migrs(0);

				num_migrs = this->migrate(deployment);

				migr_count_ += num_migrs;
				// For migration rate, use the weighted harmonic mean (see the "Workload Book" by Feitelson)
				migr_rate_den_ += num_vms;
				migr_rate_num_ += num_migrs;

//			physical_machine_id_map inactive_pms;
//
//			// Populate the inactive PMs container with all powered-on machines
//			{
//				physical_machine_container active_pms(dc.physical_machines(powered_on_power_status));
//				physical_machine_iterator pm_end_it(active_pms.end());
//				for (physical_machine_iterator pm_it = active_pms.begin(); pm_it != pm_end_it; ++pm_it)
//				{
//					physical_machine_pointer ptr_pm(*pm_it);
//
//					inactive_pms[ptr_pm->id()] = ptr_pm;
//				}
//			}
//
//			// Migrate VMs
//			{
////				dc.displace_active_virtual_machines(false);
//
//				physical_virtual_machine_map pm_vm_map(ptr_solver_->result().placement());
////[XXX]
//::std::cerr << "CHECK SOLVER PLACEMENT" << ::std::endl;//XXX
//for (typename physical_virtual_machine_map::const_iterator it = ptr_solver_->result().placement().begin(); it != ptr_solver_->result().placement().end(); ++it)//XXX
//{//XXX
//::std::cerr << "VM ID: " << (it->first.second) << " placed on PM ID: " << (it->first.first) << " with SHARE: " << ((it->second)[0].second) << ::std::endl;//XXX
//}//XXX
////[/XXX]
//				physical_virtual_machine_iterator pm_vm_end_it(pm_vm_map.end());
//				for (physical_virtual_machine_iterator pm_vm_it = pm_vm_map.begin(); pm_vm_it != pm_vm_end_it; ++pm_vm_it)
//				{
//					physical_machine_pointer ptr_pm(dc.physical_machine_ptr(pm_vm_it->first.first));
//					virtual_machine_pointer ptr_vm(dc.virtual_machine_ptr(pm_vm_it->first.second));
//					resource_share_container shares(pm_vm_it->second);
//
//					// check: paranoid check
//					DCS_DEBUG_ASSERT( ptr_pm );
//					// check: paranoid check
//					DCS_DEBUG_ASSERT( ptr_vm );
//
////::std::cerr << "Going to migrate VM (" << pm_vm_it->first.second << "): " << *ptr_vm << " into PM (" << pm_vm_it->first.first << "): " << *ptr_pm << ::std::endl; //XXX
//					if (ptr_vm->vmm().hosting_machine().id() != ptr_pm->id())
//					{
//						++migr_count_;
//					}
//
//					dc.migrate_virtual_machine(ptr_vm, ptr_pm, shares.begin(), shares.end());
//
//					if (inactive_pms.count(ptr_pm->id()) > 0)
//					{
//						inactive_pms.erase(ptr_pm->id());
//					}
//				}
//			}
//
//			// Turn off unused PMs
//			{
//				physical_machine_id_iterator pm_id_end_it(inactive_pms.end());
//				for (physical_machine_id_iterator pm_id_it = inactive_pms.begin(); pm_id_it != pm_id_end_it; ++pm_id_it)
//				{
//					physical_machine_pointer ptr_pm(pm_id_it->second);
//
//					ptr_pm->power_off();
//				}
//			}
			}
			else
			{
				log_warn(DCS_DES_CLOUD_LOGGING_AT, "Failed to solve optimization problem. Skip migration.");
				++fail_count_;
			}
		}
		else
		{
			// No VMs -> Turn off active PMs

			log_warn(DCS_DES_CLOUD_LOGGING_AT, "No VM is running. Power-off active PMs.");

			physical_machine_container pms(dc.physical_machines(powered_on_power_status));
			physical_machine_iterator pm_end_it(pms.end());
			for (physical_machine_iterator pm_it = pms.begin(); pm_it != pm_end_it; ++pm_it)
			{
				physical_machine_pointer ptr_pm(*pm_it);

				// paranoid-check: null
				DCS_DEBUG_ASSERT( ptr_pm );

				ptr_pm->power_off();
			}
		}

{//XXX
::std::time_t t(::std::time(0));//XXX
::std::string st(::std::asctime(::std::localtime(&t)));
::std::cerr << "[optimal_migration_controller] END Do Process CONTROL event (Clock: " << ctx.simulated_time() << " - Count: " << count_ << " - Fail-Count: " << fail_count_ << " - #Migrations: " << migr_count_ << " - Real-Clock: " << st.substr(0, st.size()-1) <<  " (" << static_cast< unsigned long >(t) << " secs since the Epoch" << "))" << ::std::endl;//XXX
}//XXX
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



	private: real_type wp_;
	private: real_type wm_;
	private: real_type ws_;
	private: real_type ewma_smooth_;
	private: uint_type count_;
	private: uint_type fail_count_;
	private: uint_type migr_count_;
	private: uint_type migr_rate_num_;
	private: uint_type migr_rate_den_;
	private: statistic_pointer ptr_cost_;
	private: statistic_pointer ptr_num_migr_;
	private: statistic_pointer ptr_migr_rate_;
	private: optimal_solver_pointer ptr_solver_;
	private: virtual_machine_utilization_map vm_util_map_;
}; // optimal_migration_controller

template <typename TraitsT>
const typename optimal_migration_controller<TraitsT>::real_type optimal_migration_controller<TraitsT>::default_power_cost_weight(1);

template <typename TraitsT>
const typename optimal_migration_controller<TraitsT>::real_type optimal_migration_controller<TraitsT>::default_migration_cost_weight(1);

template <typename TraitsT>
const typename optimal_migration_controller<TraitsT>::real_type optimal_migration_controller<TraitsT>::default_sla_cost_weight(1);

template <typename TraitsT>
const typename optimal_migration_controller<TraitsT>::real_type optimal_migration_controller<TraitsT>::default_ewma_smoothing_factor(0.90);

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_OPTIMAL_MIGRATION_CONTROLLER_HPP
