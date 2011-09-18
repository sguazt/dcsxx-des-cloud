#ifndef DCS_EESIM_DATA_CENTER_MANAGER_HPP
#define DCS_EESIM_DATA_CENTER_MANAGER_HPP


#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/base_initial_placement_strategy.hpp>
#include <dcs/eesim/base_migration_controller.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <iostream>


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
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
//	private: typedef ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
//	private: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;


	public: data_center_manager()
	: ptr_sys_start_evt_src_(new des_event_source_type())
	{
		init();
	}


	public: virtual ~data_center_manager()
	{
		deregister_event_handlers();
	}


	public: void controlled_data_center(data_center_pointer const& ptr_dc)
	{
		DCS_DEBUG_ASSERT( ptr_dc );

		ptr_dc_ = ptr_dc;
	}


	public: void migration_controller(migration_controller_pointer const& ptr_migrator)
	{
		DCS_DEBUG_ASSERT( ptr_migrator );

		ptr_migrator_ = ptr_migrator;
		ptr_migrator_->controlled_data_center(ptr_dc_);
	}


	public: migration_controller_type const& migration_controller() const
	{
		DCS_DEBUG_ASSERT( ptr_migrator_ );

		return *ptr_migrator_;
	}


	public: void initial_placement_strategy(initial_placement_strategy_pointer const& ptr_strategy)
	{
		ptr_init_placement_ = ptr_strategy;
	}


	private: void init()
	{
		register_event_handlers();
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


	private: void schedule_system_startup()
	{
		registry<traits_type>& reg(registry<traits_type>::instance());

		reg.des_engine().schedule_event(
				ptr_sys_start_evt_src_,
				reg.des_engine().simulated_time()
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


	private: data_center_pointer ptr_dc_;
	private: initial_placement_strategy_pointer ptr_init_placement_;
	private: migration_controller_pointer ptr_migrator_;
	private: des_event_source_pointer ptr_sys_start_evt_src_;
}; // data_center_manager


}} // Namespace dcs::eesim


#endif // DCS_EESIM_DATA_CENTER_MANAGER_HPP
