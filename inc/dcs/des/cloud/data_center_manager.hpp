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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_EESIM_DATA_CENTER_MANAGER_HPP
#define DCS_EESIM_DATA_CENTER_MANAGER_HPP


#include <cmath>
#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/application_instance.hpp>
#include <dcs/eesim/base_application_instance_builder.hpp>
#include <dcs/eesim/base_incremental_placement_strategy.hpp>
#include <dcs/eesim/base_initial_placement_strategy.hpp>
#include <dcs/eesim/base_migration_controller.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/logging.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>
#include <dcs/exception.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <iostream>
#include <sstream>
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
	public: typedef base_application_instance_builder<traits_type> application_instance_builder_type;
	public: typedef ::dcs::shared_ptr<application_instance_builder_type> application_instance_builder_pointer;
	public: typedef application_instance<traits_type> application_instance_type;
	public: typedef ::dcs::shared_ptr<application_instance_type> application_instance_pointer;
	public: typedef base_incremental_placement_strategy<traits_type> incremental_placement_strategy_type;
	public: typedef ::dcs::shared_ptr<incremental_placement_strategy_type> incremental_placement_strategy_pointer;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	private: typedef typename traits_type::real_type real_type;
	private: typedef typename traits_type::uint_type uint_type;
	private: typedef typename traits_type::application_identifier_type application_identifier_type;
	private: typedef ::std::vector<application_instance_builder_pointer> application_instance_builder_container;
	private: typedef virtual_machine<traits_type> virtual_machine_type;
	private: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	private: typedef ::std::map<application_instance_pointer,bool> application_instance_container;


	/// Default constructor.
	public: data_center_manager()
	: ptr_sys_start_evt_src_(new des_event_source_type("Data Center Start-Up")),
	  ptr_sys_stop_evt_src_(new des_event_source_type("Data Center Stop")),
	  ptr_app_start_evt_src_(new des_event_source_type("Application Starting")),
	  ptr_app_stop_evt_src_(new des_event_source_type("Application Stopping")),
	  created_(false)
	{
		init();
	}


	/// Copy constructor.
	private: data_center_manager(data_center_manager const& that)
	{
		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: data_center_manager& operator=(data_center_manager const& rhs)
	{
		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
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
#ifdef DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
		ptr_dc_->manager(this);
#endif // DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
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


#ifdef DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
	public: migration_controller_type& migration_controller()
	{
		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_migrator_ );

		return *ptr_migrator_;
	}
#endif // DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS


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

		// Register to external events
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
		// Register to internal events
		ptr_sys_start_evt_src_->connect(
				::dcs::functional::bind(
					&self_type::process_system_startup,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		ptr_sys_stop_evt_src_->connect(
				::dcs::functional::bind(
					&self_type::process_system_stop,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		ptr_app_start_evt_src_->connect(
				::dcs::functional::bind(
					&self_type::process_application_starting,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		ptr_app_stop_evt_src_->connect(
				::dcs::functional::bind(
					&self_type::process_application_stopping,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
	}


	private: void deregister_event_handlers()
	{
		registry<traits_type>& reg(registry<traits_type>::instance());

		// Deregister from external events
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
		reg.des_engine().begin_of_sim_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_begin_of_sim,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		// Register to internal events
		ptr_sys_start_evt_src_->disconnect(
				::dcs::functional::bind(
					&self_type::process_system_startup,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		ptr_sys_stop_evt_src_->disconnect(
				::dcs::functional::bind(
					&self_type::process_system_stop,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		ptr_app_start_evt_src_->disconnect(
				::dcs::functional::bind(
					&self_type::process_application_starting,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
		ptr_app_stop_evt_src_->disconnect(
				::dcs::functional::bind(
					&self_type::process_application_stopping,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);
	}


/*
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
::std::cerr << "INSTANTIATING PREALLOCATED APPLICATION: #" << i << ::std::endl;//XXX
				application_instance_pointer ptr_inst((*ptr_builder)(urng, true, cur_time));

				// paranoid-check: valid pointer
				DCS_DEBUG_ASSERT( ptr_inst );

::std::cerr << "CREATING PREALLOCATED APPLICATION: " << *(ptr_inst->application_ptr()) << " (" << ptr_inst->application_ptr() << ")" << ::std::endl;//XXX
				// Change application name to be more informative
				::std::ostringstream oss;
				oss << ptr_inst->application().name() << " (Instance #" << i << ")";
				ptr_inst->application().name(oss.str());

				ptr_dc_->add_application(ptr_inst->application_ptr(),
										 ptr_inst->application_controller_ptr());

::std::cerr << "CREATED PREALLOCATED APPLICATION: " << *(ptr_inst->application_ptr()) << " (" << ptr_inst->application_ptr() << ")" << ::std::endl;//XXX
				schedule_application_stopping(ptr_inst);
			}
			// Build "future" apps (creation is done on a scheduled event)
			num_insts = ptr_builder->min_num_instances();
			for (uint_type i = ptr_builder->num_preallocated_instances()+1; i <= num_insts; ++i)
			{
::std::cerr << "INSTANTIATING DYNAMIC APPLICATION: #" << i << ::std::endl;//XXX
				application_instance_pointer ptr_inst((*ptr_builder)(urng, false, cur_time));

				// paranoid-check: valid pointer
				DCS_DEBUG_ASSERT( ptr_inst );

::std::cerr << "SCHEDULING DYNAMIC APPLICATION: " << *(ptr_inst->application_ptr()) << " (" << ptr_inst->application_ptr() << ")" << ::std::endl;//XXX
				// Change application name to be more informative
				::std::ostringstream oss;
				oss << ptr_inst->application().name() << " (Instance #" << i << ")";
				ptr_inst->application().name(oss.str());

::std::cerr << "SCHEDULED DYNAMIC APPLICATION: " << *(ptr_inst->application_ptr()) << " (" << ptr_inst->application_ptr() << ")" << ::std::endl;//XXX
				schedule_application_creation(ptr_inst);
			}
		}
	}
*/


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
::std::cerr << "INSTANTIATING PREALLOCATED APPLICATION: #" << i << ::std::endl;//XXX
				application_instance_pointer ptr_inst((*ptr_builder)(urng, true, cur_time));

				// paranoid-check: valid pointer
				DCS_DEBUG_ASSERT( ptr_inst );

::std::cerr << "CREATING PREALLOCATED APPLICATION: " << *(ptr_inst->application_ptr()) << " (" << ptr_inst->application_ptr() << ")" << ::std::endl;//XXX
				// Change application name to be more informative
				::std::ostringstream oss;
				oss << ptr_inst->application().name() << " (Instance #" << i << ")";
				ptr_inst->application().name(oss.str());

				ptr_dc_->add_application(ptr_inst->application_ptr(),
										 ptr_inst->application_controller_ptr());

::std::cerr << "CREATED PREALLOCATED APPLICATION: " << *(ptr_inst->application_ptr()) << " (" << ptr_inst->application_ptr() << ")" << ::std::endl;//XXX
				app_insts_[ptr_inst] = true;
			}
			// Build "future" apps (creation is done on a scheduled event)
			num_insts = ptr_builder->min_num_instances();
			for (uint_type i = ptr_builder->num_preallocated_instances()+1; i <= num_insts; ++i)
			{
::std::cerr << "INSTANTIATING DYNAMIC APPLICATION: #" << i << ::std::endl;//XXX
				application_instance_pointer ptr_inst((*ptr_builder)(urng, false, cur_time));

				// paranoid-check: valid pointer
				DCS_DEBUG_ASSERT( ptr_inst );

::std::cerr << "CREATING DYNAMIC APPLICATION: " << *(ptr_inst->application_ptr()) << " (" << ptr_inst->application_ptr() << ")" << ::std::endl;//XXX
				// Change application name to be more informative
				::std::ostringstream oss;
				oss << ptr_inst->application().name() << " (Instance #" << i << ")";
				ptr_inst->application().name(oss.str());

				ptr_dc_->add_application(ptr_inst->application_ptr(),
										 ptr_inst->application_controller_ptr());
::std::cerr << "CREATED DYNAMIC APPLICATION: " << *(ptr_inst->application_ptr()) << " (" << ptr_inst->application_ptr() << ")" << ::std::endl;//XXX
				ptr_dc_->inhibit_application(ptr_inst->application_ptr()->id(), true);
				app_insts_[ptr_inst] = false;
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


	private: void schedule_system_stop()
	{
		registry<traits_type>& reg(registry<traits_type>::instance());

		reg.des_engine().schedule_event(
				ptr_sys_stop_evt_src_,
				reg.des_engine().simulated_time()
			);
::std::cerr << "data_center_manager>> Scheduled System Stop at " << reg.des_engine().simulated_time() << ::std::endl;//XXX
	}


/*
	private: void schedule_application_creation(application_instance_pointer const& ptr_app_inst)
	{
::std::cerr << "data_center_manager>> Create application: start-time: " << ptr_app_inst->start_time() << " - stop-time: " << ptr_app_inst->stop_time() << ::std::endl;//XXX
		registry<traits_type>& reg(registry<traits_type>::instance());

		real_type cur_time(reg.des_engine().simulated_time());

		if (::dcs::math::float_traits<real_type>::essentially_equal(ptr_app_inst->start_time(), cur_time))
		{
::std::cerr << "data_center_manager>> Creating application with deploy" << ::std::endl;//XXX
			ptr_dc_->add_application(ptr_app_inst->application_ptr(),
									 ptr_app_inst->application_controller_ptr(),
									 true);
::std::cerr << "data_center_manager>> Created application with deploy" << ::std::endl;//XXX
		}
		else
		{
::std::cerr << "data_center_manager>> Creating application without deploy" << ::std::endl;//XXX
			ptr_dc_->add_application(ptr_app_inst->application_ptr(),
									 ptr_app_inst->application_controller_ptr(),
//									 false);
									 true);
::std::cerr << "data_center_manager>> Created application without deploy" << ::std::endl;//XXX

			ptr_dc_->inhibit_application(ptr_app_inst->application_ptr()->id(), true);

			reg.des_engine().schedule_event(
					ptr_app_start_evt_src_,
					ptr_app_inst->start_time(),
					ptr_app_inst
				);
		}
	}
*/


	private: void schedule_application_starting(application_instance_pointer const& ptr_app_inst)
	{
		if (app_insts_.at(ptr_app_inst))
		{
			// Preallocated application are started at the beginning of the experiment
			return;
		}

::std::cerr << "data_center_manager>> Scheduling Start Application: APP: " << *(ptr_app_inst->application_ptr()) << " - start-time: " << ptr_app_inst->start_time() << " - stop-time: " << ptr_app_inst->stop_time() << ::std::endl;//XXX
		registry<traits_type>& reg(registry<traits_type>::instance());

		reg.des_engine().schedule_event(
				ptr_app_start_evt_src_,
				ptr_app_inst->start_time(),
				ptr_app_inst
			);
	}


	private: void schedule_application_stopping(application_instance_pointer const& ptr_app_inst)
	{
::std::cerr << "data_center_manager>> Scheduling Stop Application: APP: " << *(ptr_app_inst->application_ptr()) << " - start-time: " << ptr_app_inst->start_time() << " - stop-time: " << ptr_app_inst->stop_time() << ::std::endl;//XXX
		if (app_insts_.at(ptr_app_inst) || ::std::isinf(ptr_app_inst->stop_time()))
		{
			// Preallocated application are stopped at the end of the experiment
			return;
		}

		registry<traits_type>& reg(registry<traits_type>::instance());

		reg.des_engine().schedule_event(
				ptr_app_stop_evt_src_,
				ptr_app_inst->stop_time(),
				ptr_app_inst
			);
	}


	private: void process_begin_of_sim(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing BEGIN-OF-SIMULATION (Clock: " << ctx.simulated_time() << ")");//XXX

::std::cerr << "[data_center_manager>> Create applications" << ::std::endl;
		create_applications();

		DCS_DEBUG_TRACE("(" << this << ") END Processing BEGIN-OF-SIMULATION (Clock: " << ctx.simulated_time() << ")");//XXX
	}


	private: void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");//XXX

		schedule_system_startup();

		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");//XXX
	}


	private: void process_sys_finit(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");//XXX
::std::cerr << "[data_center_manager] (" << this << ") BEGIN Processing SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX

/*
		// precondition: pointer to vm initial placer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dc_ );

		// Remove all placed VMs
::std::cerr << "[data_center_manager] (" << this << ") Displacing VMs" << ::std::endl;//XXX
		ptr_dc_->displace_virtual_machines();
::std::cerr << "[data_center_manager] (" << this << ") Displaced VMs" << ::std::endl;//XXX

		typename traits_type::uint_type stopped_apps;

::std::cerr << "[data_center_manager] (" << this << ") Stopping Apps" << ::std::endl;//XXX
		stopped_apps = ptr_dc_->stop_applications();
::std::cerr << "[data_center_manager] (" << this << ") Stopped Apps" << ::std::endl;//XXX

		if (!stopped_apps)
		{
			registry<traits_type>::instance().des_engine_ptr()->stop_now();

			log_warn(DCS_EESIM_LOGGING_AT, "Unable to stop any application.");
		}
*/

//		schedule_system_stop();
		process_system_stop(evt, ctx);

::std::cerr << "[data_center_manager] (" << this << ") END Processing SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX
		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-FINALIZATION (Clock: " << ctx.simulated_time() << ")");//XXX
	}


	private: void process_system_startup(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-STARTUP (Clock: " << ctx.simulated_time() << ")");//XXX
::std::cerr << "(" << this << ") BEGIN Processing SYSTEM-STARTUP (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX

		// precondition: pointer to vm initial placer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dc_ );

//		// Remove all previously placed VM
//		ptr_dc_->displace_virtual_machines();

//::std::cerr << "[data_center_manager>> Create applications" << ::std::endl;
//		if (!created_)
//		{
//			create_applications();
//			created_ = true;
//		}

		// Create a new VM placement
::std::cerr << "[data_center_manager>> Place VMs: #" << ptr_dc_->active_virtual_machines().size() << ::std::endl;
		ptr_dc_->place_virtual_machines(
			ptr_init_placement_->placement(*ptr_dc_)
		);

		typename traits_type::uint_type started_apps;

::std::cerr << "[data_center_manager>> Start preallocated applications" << ::std::endl;
		started_apps = ptr_dc_->start_applications();

		if (!started_apps)
		{
			log_warn(DCS_EESIM_LOGGING_AT, "Unable to start any application.");

			registry<traits_type>::instance().des_engine_ptr()->stop_now();

			return;
		}

//[EXP]
		// Schedule the starting of non-preallocated apps
		typedef typename application_instance_container::const_iterator app_inst_iterator;
		app_inst_iterator app_end_it(app_insts_.end());
		for (app_inst_iterator app_it = app_insts_.begin(); app_it != app_end_it; ++app_it)
		{
			application_instance_pointer ptr_inst(app_it->first);
			bool preallocated(app_it->second);

			if (!preallocated)
			{
				schedule_application_starting(ptr_inst);
			}
		}
//[/EXP]

::std::cerr << "(" << this << ") END Processing SYSTEM-STARTUP (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX
		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-STARTUP (Clock: " << ctx.simulated_time() << ")");//XXX
	}


	private: void process_system_stop(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		//DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-STOP (Clock: " << ctx.simulated_time() << ")");//XXX
::std::cerr << "(" << this << ") BEGIN Processing SYSTEM-STOP (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX

		// precondition: pointer to vm initial placer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_dc_ );

::std::cerr << "[data_center_manager>> Stop applications" << ::std::endl;
		typename traits_type::uint_type stopped_apps;

		stopped_apps = ptr_dc_->stop_applications();
		if (!stopped_apps)
		{
			log_warn(DCS_EESIM_LOGGING_AT, "Unable to stop any application.");

			registry<traits_type>::instance().des_engine_ptr()->stop_now();

			return;
		}

::std::cerr << "[data_center_manager>> Displace applications" << ::std::endl;
		// Remove all previously placed VM
		ptr_dc_->displace_virtual_machines();

//		undeploy_applications();
//		destroy_applications();

::std::cerr << "(" << this << ") END Processing SYSTEM-STOP (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX
		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-STOP (Clock: " << ctx.simulated_time() << ")");//XXX
	}


    private: void process_application_starting(des_event_type const& evt, des_engine_context_type& ctx)
    {
		typedef ::std::vector<virtual_machine_pointer> vm_container;
		typedef virtual_machines_placement<traits_type> vms_placement_type;
		typedef typename vms_placement_type::const_iterator vms_placement_iterator;

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing APPLICATION-CREATION (Clock: " << ctx.simulated_time() << ")");//XXX
::std::cerr << "[data_center_manager] (" << this << ") BEGIN Processing APPLICATION-CREATION (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX

		application_instance_pointer ptr_app_inst(evt.template unfolded_state<application_instance_pointer>());

		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_app_inst );

::std::cerr << "[data_center_manager] DEPLOYING DYNAMIC APPLICATION: " << *(ptr_app_inst->application_ptr()) << ::std::endl;//XXX
		application_identifier_type app_id(ptr_app_inst->application().id());

		ptr_dc_->inhibit_application(app_id, false);

		// Create VMs
		ptr_dc_->deploy_application(app_id);

		// Place VMs
		vm_container vms(ptr_dc_->application_virtual_machines(app_id));
		vms_placement_type vm_placement;
		vm_placement = ptr_incr_placement_->place(*ptr_dc_, vms.begin(), vms.end());
		vms_placement_iterator vmp_end_it(vm_placement.end());
		for (vms_placement_iterator vmp_it = vm_placement.begin(); vmp_it != vmp_end_it; ++vmp_it)
		{
			ptr_dc_->place_virtual_machine(
					ptr_dc_->virtual_machine_ptr(vm_placement.vm_id(vmp_it)),
					ptr_dc_->physical_machine_ptr(vm_placement.pm_id(vmp_it)),
					vm_placement.shares_begin(vmp_it),
					vm_placement.shares_end(vmp_it),
					false
				);
		}

		// Start application
		bool started(false);
		started = ptr_dc_->start_application(app_id);
::std::cerr << "[data_center_manager] STARTING DYNAMIC APPLICATION: " << *(ptr_app_inst->application_ptr()) << " --> " << ::std::boolalpha << started << ::std::endl;//XXX
		if (!started)
		{
			::std::ostringstream oss;
			oss << "Application '" << app_id << "' cannot be started.";

			DCS_EXCEPTION_THROW( ::std::runtime_error, oss.str() );
		}

		// Schedule application stop event
		schedule_application_stopping(ptr_app_inst);

::std::cerr << "[data_center_manager] (" << this << ") END Processing APPLICATION-CREATION (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX
		DCS_DEBUG_TRACE("(" << this << ") END Processing APPLICATION-CREATION (Clock: " << ctx.simulated_time() << ")");//XXX
	}


	private: void process_application_stopping(des_event_type const& evt, des_engine_context_type& ctx)
	{
		typedef ::std::vector<virtual_machine_pointer> vm_container;
		typedef typename vm_container::const_iterator vm_iterator;

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing APPLICATION-STOPPING (Clock: " << ctx.simulated_time() << ")");//XXX
::std::cerr << "[data_center_manager] (" << this << ") BEGIN Processing APPLICATION-STOPPING (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX

		application_instance_pointer ptr_app_inst(evt.template unfolded_state<application_instance_pointer>());

		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_app_inst );

		application_identifier_type app_id(ptr_app_inst->application().id());

		// Stop application
		ptr_dc_->stop_application(app_id);

//		// Destroy VMs
//		ptr_dc_->undeploy_application(app_id);

//		// Remove VMs from physical machines
		vm_container vms(ptr_dc_->application_virtual_machines(app_id));
		vm_iterator vm_end_it(vms.end());
		for (vm_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_pointer ptr_vm(*vm_it);

			// paranoid-check: valid pointer.
			DCS_DEBUG_ASSERT( ptr_vm );

			ptr_dc_->displace_virtual_machine(ptr_vm);
		}

		ptr_dc_->inhibit_application(app_id, true);

::std::cerr << "[data_center_manager] (" << this << ") END Processing APPLICATION-STOPPING (Clock: " << ctx.simulated_time() << ")" << ::std::endl;//XXX
		DCS_DEBUG_TRACE("(" << this << ") END Processing APPLICATION-STOPPING (Clock: " << ctx.simulated_time() << ")");//XXX
	}


	private: data_center_pointer ptr_dc_;
	private: initial_placement_strategy_pointer ptr_init_placement_;
	private: incremental_placement_strategy_pointer ptr_incr_placement_;
	private: migration_controller_pointer ptr_migrator_;
	private: application_instance_builder_container app_builders_;
	private: des_event_source_pointer ptr_sys_start_evt_src_;
	private: des_event_source_pointer ptr_sys_stop_evt_src_;
	private: des_event_source_pointer ptr_app_start_evt_src_;
	private: des_event_source_pointer ptr_app_stop_evt_src_;
	private: bool created_;//EXP
	private: application_instance_container app_insts_;//EXP
}; // data_center_manager

}} // Namespace dcs::eesim


#endif // DCS_EESIM_DATA_CENTER_MANAGER_HPP
