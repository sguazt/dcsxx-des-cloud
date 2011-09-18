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
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <string>


namespace dcs { namespace eesim {

template <typename TraitsT>
class base_migration_controller
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
			this->disconnect_from_event_sources();

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
		this->disconnect_from_event_sources();
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


	protected: data_center_pointer controlled_data_center_ptr() const
	{
		return ptr_dc_;
	}


	protected: data_center_pointer controlled_data_center_ptr()
	{
		return ptr_dc_;
	}


	private: void init()
	{
		this->connect_to_event_sources();
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

			reg.des_engine().system_initialization_event_source().connect(
					::dcs::functional::bind(
						&self_type::process_sys_init,
						this,
						::dcs::functional::placeholders::_1,
						::dcs::functional::placeholders::_2
					)
				);
			reg.des_engine().system_initialization_event_source().connect(
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

	private: void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		schedule_control();

		do_process_sys_init(evt, ctx);
	}


	private: void process_sys_finit(des_event_type const& evt, des_engine_context_type& ctx)
	{
		do_process_sys_finit(evt, ctx);
	}


	private: void process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		schedule_control();

		do_process_control(evt, ctx);
	}

	//@} Event Handlers


	//@{ Interface Member Functions

	protected: virtual void do_controlled_data_center(data_center_pointer const& ptr_dc)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_dc );

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


	private: virtual void do_process_control(des_event_type const& evt, des_engine_context_type& ctx) = 0;


	private: virtual statistic_type const& do_num_migrations() const = 0;


	//@} Interface Member Functions


	private: data_center_pointer ptr_dc_;
	private: real_type ts_;
	private: des_event_source_pointer ptr_control_evt_src_;
}; // base_migration_controller

template <typename TraitsT>
const ::std::string base_migration_controller<TraitsT>::control_event_source_name("Control Data Center");

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_MIGRATION_CONTROLLER_HPP
