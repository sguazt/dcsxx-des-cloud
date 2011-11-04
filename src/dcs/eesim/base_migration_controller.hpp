/**
 * \file src/dcs/eesim/base_migration_controller.hpp
 *
 * \brief Base class for migration controllers.
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

#ifndef DCS_EESIM_BASE_MIGRATION_CONTROLLER_HPP
#define DCS_EESIM_BASE_MIGRATION_CONTROLLER_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/entity.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <map>
#include <string>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class base_migration_controller: public ::dcs::des::entity
{
	private: typedef base_migration_controller<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef data_center<traits_type> data_center_type;
	public: typedef ::dcs::shared_ptr<data_center_type> data_center_pointer;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	public: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	public: typedef ::dcs::des::base_statistic<real_type,uint_type> statistic_type;
	protected: typedef virtual_machines_placement<traits_type> virtual_machines_placement_type;
//	protected: typedef typename traits_type::physical_machine_identifier_type physical_machine_identifier_type;
//	protected: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
//	protected: typedef ::std::map<physical_resource_category,real_type> resource_share_container;
//	protected: typedef ::std::pair<virtual_machine_identifier_type,physical_machine_identifier_type> physical_virtual_machine_pair;
//	protected: typedef ::std::map<physical_virtual_machine_pair,resource_share_container> physical_virtual_machine_map;
	private: typedef registry<traits_type> registry_type;


	private: static const ::std::string control_event_source_name;



	/// Default constructor.
	protected: base_migration_controller()
	: ptr_dc_(),
	  ts_(0),
	  ptr_control_evt_src_(new des_event_source_type(control_event_source_name))
	{
		init();
	}


	/// A constructor.
	protected: base_migration_controller(real_type ts)
	: ptr_dc_(),
	  ts_(ts),
	  ptr_control_evt_src_(new des_event_source_type(control_event_source_name))
	{
		init();
	}


	/// A constructor.
	protected: base_migration_controller(data_center_pointer const& ptr_dc, real_type ts)
	: ptr_dc_(ptr_dc),
	  ts_(ts),
	  ptr_control_evt_src_(new des_event_source_type(control_event_source_name))
	{
		init();
	}


	/// Copy constructor.
	public: base_migration_controller(base_migration_controller const& that)
	: ptr_dc_(that.ptr_dc_),
	  ts_(that.ts_),
	  ptr_control_evt_src_(new des_event_source_type(*that.ptr_control_evt_src_))
	{
		init();
	}

	/// Copy assignment.
	public: base_migration_controller& operator=(base_migration_controller const& rhs)
	{
		if (this != &rhs)
		{
			finit();

			ptr_dc_ = rhs.ptr_dc_;
			ts_ = rhs.ts_;
			ptr_control_evt_src_ = ::dcs::make_shared<des_event_source_type>(*(rhs.ptr_control_evt_src_));

			init();
		}

		return *this;
	}


	/// The destructor.
	public: virtual ~base_migration_controller()
	{
		finit();
	}


	public: void controlled_data_center(data_center_pointer const& ptr_dc)
	{
		do_controlled_data_center(ptr_dc);

		ptr_dc_ = ptr_dc;
	}


	public: data_center_type& controlled_data_center()
	{
		// pre: data center must have been set
		DCS_DEBUG_ASSERT( ptr_dc_ );

		return *ptr_dc_;
	}


	public: data_center_type const& controlled_data_center() const
	{
		// pre: data center must have been set
		DCS_DEBUG_ASSERT( ptr_dc_ );

		return *ptr_dc_;
	}


	public: des_event_source_type& control_event_source()
	{
		return *ptr_control_evt_src_;
	}


	public: des_event_source_type const& control_event_source() const
	{
		return *ptr_control_evt_src_;
	}


	public: void sampling_time(real_type ts)
	{
		ts_ = ts;
	}


	public: real_type sampling_time() const
	{
		return ts_;
	}


	public: statistic_type const& num_migrations() const
	{
		return do_num_migrations();
	}


	public: statistic_type const& migration_rate() const
	{
		return do_migration_rate();
	}


	protected: data_center_pointer controlled_data_center_ptr() const
	{
		return ptr_dc_;
	}


	protected: data_center_pointer controlled_data_center_ptr()
	{
		return ptr_dc_;
	}


	protected: uint_type migrate(virtual_machines_placement_type const& placement)
	{
		typedef typename traits_type::physical_machine_identifier_type pm_identifier_type;
		typedef typename traits_type::virtual_machine_identifier_type vm_identifier_type;
		typedef typename data_center_type::physical_machine_pointer pm_pointer;
		typedef typename data_center_type::virtual_machine_pointer vm_pointer;
		typedef ::std::vector<pm_pointer> pm_container;
		typedef typename pm_container::const_iterator pm_iterator;
		typedef ::std::vector<vm_pointer> vm_container;
		typedef typename vm_container::const_iterator vm_iterator;
		typedef ::std::map<pm_identifier_type,pm_pointer> pm_id_map;
		typedef typename pm_id_map::iterator pm_id_iterator;
//		typedef typename physical_virtual_machine_map::const_iterator physical_virtual_machine_iterator;
		typedef typename virtual_machines_placement_type::const_iterator vm_placement_iterator;
		typedef ::std::map<vm_identifier_type,vm_pointer> vm_id_map;
		typedef typename vm_id_map::iterator vm_id_iterator;

		if (placement.empty())
		{
			return 0;
		}

		pm_id_map inactive_pms;
		vm_id_map inactive_vms;
		uint_type num_migrs(0);

		// Populate the inactive PMs container with all powered-on machines
		{
			pm_container active_pms(ptr_dc_->physical_machines(powered_on_power_status));
			pm_iterator pm_end_it(active_pms.end());
			for (pm_iterator pm_it = active_pms.begin(); pm_it != pm_end_it; ++pm_it)
			{
				pm_pointer ptr_pm(*pm_it);

				inactive_pms[ptr_pm->id()] = ptr_pm;
			}
		}

		// Populate the inactive VMs container with all active virtual machines
		{
			vm_container active_vms(ptr_dc_->active_virtual_machines());
			vm_iterator vm_end_it(active_vms.end());
			for (vm_iterator vm_it = active_vms.begin(); vm_it != vm_end_it; ++vm_it)
			{
				vm_pointer ptr_vm(*vm_it);

				inactive_vms[ptr_vm->id()] = ptr_vm;
			}
		}

		// Migrate VMs
		{
			//NOTE: we cannot use directly the method data_center::migrate for
			//      the following reason.
			//      Suppose the VM1 on PM1 is to be migrated on PM2 and that VM2
			//      on PM2 is to be migrated on PM1.
			//      If there is not enough space on PM1 (PM2), we cannot move
			//      VM2 (VM1) on it, without before powering-off VM1 (VM2).
			//      The situation complicates when there are more than one VMs
			//      involved in this circular dependencies.
			//      So to solve we first power-off and displace all VMs, and
			//      then place them on the new positions.
			//      The drawback of this is that this does not mimic the true VM
			//      live-migration since VMs are first powered-off and then
			//      powered-on (just like a cold migration).

			vm_placement_iterator pm_vm_end_it(placement.end());

			// Displace
			for (vm_placement_iterator pm_vm_it = placement.begin(); pm_vm_it != pm_vm_end_it; ++pm_vm_it)
			{
				pm_pointer ptr_pm(ptr_dc_->physical_machine_ptr(placement.pm_id(pm_vm_it)));
				vm_pointer ptr_vm(ptr_dc_->virtual_machine_ptr(placement.vm_id(pm_vm_it)));

				// check: paranoid check
				DCS_DEBUG_ASSERT( ptr_pm );
				// check: paranoid check
				DCS_DEBUG_ASSERT( ptr_vm );

				if (ptr_vm->vmm().hosting_machine().id() != ptr_pm->id())
				{
					++num_migrs;
				}

				// Power-off and displace
				ptr_dc_->displace_virtual_machine(ptr_vm, true);
			}

			// Place
			for (vm_placement_iterator pm_vm_it = placement.begin(); pm_vm_it != pm_vm_end_it; ++pm_vm_it)
			{
				pm_pointer ptr_pm(ptr_dc_->physical_machine_ptr(placement.pm_id(pm_vm_it)));
				vm_pointer ptr_vm(ptr_dc_->virtual_machine_ptr(placement.vm_id(pm_vm_it)));

				// check: paranoid check
				DCS_DEBUG_ASSERT( ptr_pm );
				// check: paranoid check
				DCS_DEBUG_ASSERT( ptr_vm );

				ptr_dc_->place_virtual_machine(ptr_vm, ptr_pm, placement.shares_begin(pm_vm_it), placement.shares_end(pm_vm_it));
				ptr_pm->vmm().power_on(ptr_vm);

				if (inactive_pms.count(ptr_pm->id()) > 0)
				{
					inactive_pms.erase(ptr_pm->id());
				}
			}
		}

		// Turn off unused VMs
		{
			vm_id_iterator vm_id_end_it(inactive_vms.end());
			for (vm_id_iterator vm_id_it = inactive_vms.begin(); vm_id_it != vm_id_end_it; ++vm_id_it)
			{
				vm_pointer ptr_vm(vm_id_it->second);

				ptr_dc_->displace_virtual_machine(ptr_vm);
			}
		}

		// Turn off unused PMs
		{
			pm_id_iterator pm_id_end_it(inactive_pms.end());
			for (pm_id_iterator pm_id_it = inactive_pms.begin(); pm_id_it != pm_id_end_it; ++pm_id_it)
			{
				pm_pointer ptr_pm(pm_id_it->second);

				ptr_pm->power_off();
			}
		}

		return num_migrs;
	}


	private: void init()
	{
		this->connect_to_event_sources();
	}


	private: void finit()
	{
		this->disconnect_from_event_sources();
	}


	private: void connect_to_event_sources()
	{
		if (ts_ > 0)
		{
			ptr_control_evt_src_->connect(
				::dcs::functional::bind(
					&self_type::process_control,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);

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
	}


	private: void disconnect_from_event_sources()
	{
		if (ts_ > 0)
		{
			if (ptr_control_evt_src_)
			{
				ptr_control_evt_src_->disconnect(
					::dcs::functional::bind(
						&self_type::process_control,
						this,
						::dcs::functional::placeholders::_1,
						::dcs::functional::placeholders::_2
					)
				);
			}

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
			reg.des_engine().system_initialization_event_source().disconnect(
					::dcs::functional::bind(
						&self_type::process_sys_finit,
						this,
						::dcs::functional::placeholders::_1,
						::dcs::functional::placeholders::_2
					)
				);
		}
	}


	//@{ Event Triggers

	private: void schedule_control()
	{
		if (ts_ > 0)
		{
			registry_type& reg(registry_type::instance());

			reg.des_engine().schedule_event(
				ptr_control_evt_src_,
				reg.des_engine().simulated_time() + ts_
			);
		}

		do_schedule_control();
	}

	//@} Event Triggers


	//@{ Event Handlers

	private: void process_begin_of_sim(des_event_type const& evt, des_engine_context_type& ctx)
	{
		do_process_begin_of_sim(evt, ctx);
	}


	private: void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		if (this->enabled())
		{
			schedule_control();
		}

		do_process_sys_init(evt, ctx);
	}


	private: void process_sys_finit(des_event_type const& evt, des_engine_context_type& ctx)
	{
		do_process_sys_finit(evt, ctx);
	}


	private: void process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		if (this->enabled())
		{
			schedule_control();
		}

		do_process_control(evt, ctx);
	}

	//@} Event Handlers


	//@{ Interface Member Functions

	protected: virtual void do_controlled_data_center(data_center_pointer const& ptr_dc)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_dc );

		// empty
	}


	protected: virtual void do_process_begin_of_sim(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// empty
	}


	protected: virtual void do_process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// empty
	}


	protected: virtual void do_process_sys_finit(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// empty
	}


	protected: virtual void do_schedule_control()
	{
		// empty
	}


	protected: virtual void do_enable(bool flag)
	{
		ptr_control_evt_src_->enable(flag);

		if (flag && !this->enabled())
		{
			schedule_control();
		}
	}


	private: virtual void do_process_control(des_event_type const& evt, des_engine_context_type& ctx) = 0;


	private: virtual statistic_type const& do_num_migrations() const = 0;


	private: virtual statistic_type const& do_migration_rate() const = 0;

	//@} Interface Member Functions


	private: data_center_pointer ptr_dc_;
	private: real_type ts_;
	private: des_event_source_pointer ptr_control_evt_src_;
}; // base_migration_controller

template <typename TraitsT>
const ::std::string base_migration_controller<TraitsT>::control_event_source_name("Control Data Center");

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_MIGRATION_CONTROLLER_HPP
