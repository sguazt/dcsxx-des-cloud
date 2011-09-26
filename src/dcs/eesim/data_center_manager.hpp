/**
 * \file dcs/eesim/data_center_manager.hpp
 *
 * \brief Manager for a data center.
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

#ifndef DCS_EESIM_DATA_CENTER_MANAGER_HPP
#define DCS_EESIM_DATA_CENTER_MANAGER_HPP


#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/application_instance.hpp>
#include <dcs/eesim/application_instance_builder.hpp>
#include <dcs/eesim/base_incremental_placement_strategy.hpp>
#include <dcs/eesim/base_initial_placement_strategy.hpp>
#include <dcs/eesim/base_migration_controller.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class data_center_manager
{
	private: typedef data_center_manager<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef data_center<traits_type> data_center_type;
	public: typedef ::dcs::shared_ptr<data_center_type> data_center_pointer;
	public: typedef base_migration_controller<traits_type> migration_controller_type;
	public: typedef ::dcs::shared_ptr<migration_controller_type> migration_controller_pointer;
	public: typedef base_initial_placement_strategy<traits_type> initial_placement_strategy_type;
	public: typedef ::dcs::shared_ptr<initial_placement_strategy_type> initial_placement_strategy_pointer;
	//[NEW]
	public: typedef base_application_instance_builder<traits_type> application_instance_builder_type;
	public: typedef ::dcs::shared_ptr<application_instance_builder_type> application_instance_builder_pointer;
	public: typedef application_instance<traits_type> application_instance_type;
	public: typedef ::dcs::shared_ptr<application_instance_type> application_instance_pointer;
	public: typedef base_incremental_placement_strategy<traits_type> incremental_placement_strategy_type;
	public: typedef ::dcs::shared_ptr<incremental_placement_strategy_type> incremental_placement_strategy_pointer;
	//[/NEW]
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
//	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
//	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	//[NEW]
	private: typedef typename traits_type::real_type real_type;
	private: typedef typename traits_type::uint_type uint_type;
	private: typedef typename traits_type::application_identifier_type application_identifier_type;
	private: typedef ::std::vector<application_instance_builder_pointer> application_instance_builder_container;
	private: typedef virtual_machine<traits_type> virtual_machine_type;
	private: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	//[/NEW]


	/// Default constructor.
	public: data_center_manager()
	: ptr_sys_start_evt_src_(new des_event_source_type()),
	  ptr_app_creat_evt_src_(new des_event_source_type()),
	  ptr_app_destr_evt_src_(new des_event_source_type())
	{
		init();
	}


	/// Destructor.
	public: virtual ~data_center_manager()
	{
		finit();
	}


	public: void controlled_data_center(data_center_pointer const& ptr_dc)
	{
		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_dc );

		ptr_dc_ = ptr_dc;
	}


	public: void migration_controller(migration_controller_pointer const& ptr_migrator)
	{
		// paranoid-check: valid pointer
		DCS_ASSERT(
				ptr_migrator,
				throw ::std::invalid_argument("[dcs::eesim::data_center_manager::migration_controller] Invalid pointer.")
			);

		ptr_migrator_ = ptr_migrator;
		ptr_migrator_->controlled_data_center(ptr_dc_);
	}


	public: migration_controller_type const& migration_controller() const
	{
		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_migrator_ );

		return *ptr_migrator_;
	}


	public: void initial_placement_strategy(initial_placement_strategy_pointer const& ptr_strategy)
	{
		// paranoid-check: valid pointer
		DCS_ASSERT(
				ptr_strategy,
				throw ::std::invalid_argument("[dcs::eesim::data_center_manager::initial_placement_strategy] Invalid pointer.")
			);

		ptr_init_placement_ = ptr_strategy;
	}


	public: initial_placement_strategy_type const& initial_placement_strategy() const
	{
		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_init_placement_ );

		return *ptr_init_placement_;
	}


	public: void incremental_placement_strategy(incremental_placement_strategy_pointer const& ptr_strategy)
	{
		// paranoid-check: valid pointer
		DCS_ASSERT(
				ptr_strategy,
				throw ::std::invalid_argument("[dcs::eesim::data_center_manager::incremental_placement_strategy] Invalid pointer.")
			);

		ptr_incr_placement_ = ptr_strategy;
	}


	public: incremental_placement_strategy_type const& incremental_placement_strategy() const
	{
		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_incr_placement_ );

		return *ptr_incr_placement_;
	}


	public: void add_application_instance_builder(application_instance_builder_pointer const& ptr_app_builder)
	{
		// pre: application instance builder must be a valid pointer.
		DCS_ASSERT(
				ptr_app_builder,
				throw ::std::invalid_argument("[dcs::eesim::data_center_manager::add_application_instance_builder] Invalid pointer.")
			);

		app_builders_.push_back(ptr_app_builder);
	}


	private: void init()
	{
		register_event_handlers();
	}


	private: void finit()
	{
		deregister_event_handlers();
	}


	private: void register_event_handlers()
	{
		registry<traits_type>& reg(registry<traits_type>::instance());

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
		ptr_sys_start_evt_src_->connect(
				::dcs::functional::bind(
					&self_type::process_system_startup,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
	}


	private: void deregister_event_handlers()
	{
		registry<traits_type>& reg(registry<traits_type>::instance());

		reg.des_engine().system_finalization_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_sys_finit,
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
	}


	private: void create_applications()
	{
		typedef typename application_instance_builder_container::const_iterator app_build_iterator;
		typedef typename traits_type::uniform_random_generator_type urng_type;

		registry<traits_type>& reg(registry<traits_type>::instance());

		urng_type& urng(reg.uniform_random_generator());

		real_type cur_time(reg.des_engine().simulated_time());

		app_build_iterator build_end_it(app_builders_.end());
		for (app_build_iterator build_it = app_builders_.begin(); build_it != build_end_it; ++build_it)
		{
			application_instance_builder_pointer ptr_builder(*build_it);

			// Build pre-allocated apps (does not need event schedulation)
			uint_type num_insts(ptr_builder->num_preallocated_instances());
			for (uint_type i = 1; i <= num_insts; ++i)
			{
				application_instance_pointer ptr_inst((*ptr_builder)(urng, cur_time));

				// paranoid-check: valid pointer
				DCS_DEBUG_ASSERT( ptr_inst );

				ptr_dc_->add_application(ptr_inst->application_ptr(),
										 ptr_inst->application_controller_ptr());
			}
			// Build "future" apps (creation is done on a scheduled event)
			num_insts = ptr_builder->min_num_instances();
			for (uint_type i = ptr_builder->num_preallocated_instances()+1; i <= num_insts; ++i)
			{
				application_instance_pointer ptr_inst((*ptr_builder)(urng, cur_time));

				// paranoid-check: valid pointer
				DCS_DEBUG_ASSERT( ptr_inst );

				schedule_application_creation(ptr_inst);
			}
		}
	}


	private: void schedule_system_startup()
	{
		registry<traits_type>& reg(registry<traits_type>::instance());

		reg.des_engine().schedule_event(
				ptr_sys_start_evt_src_,
				reg.des_engine().simulated_time()
			);
	}


	private: void schedule_application_creation(application_instance_pointer const& ptr_app_inst)
	{
		registry<traits_type>& reg(registry<traits_type>::instance());

		real_type cur_time(reg.des_engine().simulated_time());

		if (::dcs::math::float_traits<real_type>::essentially_equal(ptr_app_inst->start_time(), cur_time))
		{
			ptr_dc_->add_application(ptr_app_inst->application_ptr(),
											  ptr_app_inst->application_controller_ptr(),
											  true);
		}
		else
		{
			ptr_dc_->add_application(ptr_app_inst->application_ptr(),
											  ptr_app_inst->application_controller_ptr(),
											  false);

			reg.des_engine().schedule_event(
					ptr_app_creat_evt_src_,
					ptr_app_inst->start_time(),
					ptr_app_inst
				);
		}
	}


	private: void schedule_application_destruction(application_instance_pointer const& ptr_app_inst)
	{
		registry<traits_type>& reg(registry<traits_type>::instance());

		reg.des_engine().schedule_event(
				ptr_app_destr_evt_src_,
				ptr_app_inst->stop_time(),
				ptr_app_inst
			);
	}


	private: void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");//XXX

//[XXX]: moved in process_system_startup
//		// precondition: pointer to vm initial placer must be a valid pointer
//		DCS_DEBUG_ASSERT( ptr_dc_ );
//
////		// Remove all previously placed VM
////		ptr_dc_->displace_virtual_machines();
//
//		// Create a new VM placement
//		ptr_dc_->place_virtual_machines(
//			ptr_init_placement_->placement(*ptr_dc_)
//		);
//
//		typename traits_type::uint_type started_apps;
//
//		started_apps = ptr_dc_->start_applications();
//
//		if (!started_apps)
//		{
//			registry<traits_type>::instance().des_engine_ptr()->stop_now();
//
//			::std::clog << "[Warning] Unable to start any application." << ::std::endl;
//		}
//[/XXX]: moved in process_system_startup

		schedule_system_startup();

		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");//XXX
	}


	private: void process_sys_finit(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");//XXX

		// precondition: pointer to vm initial placer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dc_ );

		// Remove all placed VMs
		ptr_dc_->displace_virtual_machines();

		typename traits_type::uint_type stopped_apps;

		stopped_apps = ptr_dc_->stop_applications();

		if (!stopped_apps)
		{
			registry<traits_type>::instance().des_engine_ptr()->stop_now();

			::std::clog << "[Warning] Unable to stop any application." << ::std::endl;
		}

		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");//XXX
	}


	private: void process_system_startup(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-STARTUP (Clock: " << ctx.simulated_time() << ")");//XXX

		// precondition: pointer to vm initial placer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dc_ );

//		// Remove all previously placed VM
//		ptr_dc_->displace_virtual_machines();

		create_applications();

		// Create a new VM placement
		ptr_dc_->place_virtual_machines(
			ptr_init_placement_->placement(*ptr_dc_)
		);

		typename traits_type::uint_type started_apps;

		started_apps = ptr_dc_->start_applications();

		if (!started_apps)
		{
			registry<traits_type>::instance().des_engine_ptr()->stop_now();

			::std::clog << "[Warning] Unable to start any application." << ::std::endl;
		}

		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-STARTUP (Clock: " << ctx.simulated_time() << ")");//XXX
	}


    private: void process_application_creation(des_event_type const& evt, des_engine_context_type& ctx)
    {
		typedef ::std::vector<virtual_machine_pointer> vm_container;

		application_instance_pointer ptr_app_inst(evt.template unfolded_state<application_instance_pointer>());

		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_app_inst );

		application_identifier_type app_id(ptr_app_inst->application().id());

		// Create VMs
		deploy_application(app_id);

		// Place VMs
		vm_container vms(ptr_dc_->application_virtual_machines(app_id));
		//ptr_incr_placement_->place(ptr_dc_, vms.begin(), vms.end());
		//place_virtual_machine...

		// Start application
		start_application(app_id);

		// Schedule application stop event
		schedule_application_stop(ptr_app_inst);
	}


	private: void process_application_destruction(des_event_type const& evt, des_engine_context_type& ctx)
	{
		application_instance_pointer ptr_app_inst(evt.template unfolded_state<application_instance_pointer>());

		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_app_inst );

		application_identifier_type app_id(ptr_app_inst->application().id());

		stop_application(app_id);

		//displace_virtual_machine...

		undeploy_application(app_id);
	}


	private: data_center_pointer ptr_dc_;
	private: initial_placement_strategy_pointer ptr_init_placement_;
	private: incremental_placement_strategy_pointer ptr_incr_placement_;
	private: migration_controller_pointer ptr_migrator_;
	private: application_instance_builder_container app_builders_;
	private: des_event_source_pointer ptr_sys_start_evt_src_;
	private: des_event_source_pointer ptr_app_creat_evt_src_;
	private: des_event_source_pointer ptr_app_destr_evt_src_;
}; // data_center_manager

}} // Namespace dcs::eesim


#endif // DCS_EESIM_DATA_CENTER_MANAGER_HPP
