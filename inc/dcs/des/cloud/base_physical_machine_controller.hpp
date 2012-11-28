/**
 * \file dcs/des/cloud/base_physical_machine_controller.hpp
 *
 * \brief Base class for physical machine controllers.
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

#ifndef DCS_DES_CLOUD_BASE_PHYSICAL_MACHINE_CONTROLLER_HPP
#define DCS_DES_CLOUD_BASE_PHYSICAL_MACHINE_CONTROLLER_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/entity.hpp>
#include <dcs/des/cloud/physical_machine.hpp>
#include <dcs/des/cloud/registry.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <string>


namespace dcs { namespace des { namespace cloud {

template <typename TraitsT>
class base_physical_machine_controller: public ::dcs::des::entity
{
	private: typedef base_physical_machine_controller<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef physical_machine<traits_type> physical_machine_type;
	//public: typedef physical_machine_type* physical_machine_pointer;
	public: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	public: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	private: typedef registry<traits_type> registry_type;


	private: static const ::std::string control_event_source_name;


	/// A constructor.
	protected: base_physical_machine_controller()
	: ptr_mach_(),
	  ts_(0),
//	  passive_(false),
	  ptr_control_evt_src_(/*new des_event_source_type(control_event_source_name)*/)
	{
		init();
	}


	/// A constructor.
	protected: explicit base_physical_machine_controller(physical_machine_pointer const& ptr_mach, real_type ts)
	: ptr_mach_(ptr_mach),
	  ts_(ts),
//	  passive_(false),
	  ptr_control_evt_src_(/*new des_event_source_type(control_event_source_name)*/)
	{
		init();
	}


	/// Copy constructor.
	public: base_physical_machine_controller(base_physical_machine_controller const& that)
	: ptr_mach_(that.ptr_mach_),
	  ts_(that.ts_),
//	  passive_(that.passive_)
	  ptr_control_evt_src_(that.ptr_control_evt_src_ ? new des_event_source_type(*that.ptr_control_evt_src_) : new des_event_source_type(control_event_source_name))
	{
		init();
	}


	/// Copy assignment.
	public: base_physical_machine_controller& operator=(base_physical_machine_controller const& rhs)
	{
		if (this != &rhs)
		{
			finit();

			ptr_mach_ = rhs.ptr_mach_;
			ts_ = rhs.ts_;
//			passive_ = rhs.passive;
			if (rhs.ptr_control_evt_src_)
			{
				ptr_control_evt_src_ = ::dcs::make_shared<des_event_source_type>(*(rhs.ptr_control_evt_src_));
			}
			else
			{
				ptr_control_evt_src_ = ::dcs::make_shared<des_event_source_type>(control_event_source_name);
			}

			init();
		}

		return *this;
	}


	/// The destructor.
	public: virtual ~base_physical_machine_controller()
	{
		finit();
	}


	public: void machine(physical_machine_pointer const& ptr_mach)
	{
		do_machine(ptr_mach);

		ptr_mach_ = ptr_mach;
	}


	public: physical_machine_type& machine()
	{
		// pre: ptr_mach_ must be a valid physical machine pointer.
		DCS_DEBUG_ASSERT( ptr_mach_ );

		return *ptr_mach_;
	}


	public: physical_machine_type const& machine() const
	{
		// pre: ptr_mach_ must be a valid physical machine pointer.
		DCS_DEBUG_ASSERT( ptr_mach_ );

		return *ptr_mach_;
	}


	public: des_event_source_type const& control_event_source() const
	{
		return *ptr_control_evt_src_;
	}


	public: void sampling_time(real_type ts)
	{
		this->disconnect_from_event_sources();

		ts_ = ts;

		this->connect_to_event_sources();
	}


	public: real_type sampling_time() const
	{
		return ts_;
	}


	//FIXME: [sguazt] EXP
//	public: void passive(bool value)
//	{
//		passive_ = value;
//	}


//	public: bool passive() const
//	{
//		return passive_;
//	}


	public: void control()
	{
		do_control();
	}
	//FIXME: [sguazt] EXP


	protected: physical_machine_pointer machine_ptr() const
	{
		return ptr_mach_;
	}


	protected: physical_machine_pointer machine_ptr()
	{
		return ptr_mach_;
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
			if (!ptr_control_evt_src_)
			{
				ptr_control_evt_src_ = ::dcs::make_shared<des_event_source_type>();
			}

			ptr_control_evt_src_->connect(
				::dcs::functional::bind(
					&self_type::process_control,
					this,
					::dcs::functional::placeholders::_1,
					::dcs::functional::placeholders::_2
				)
			);

			registry_type& reg(registry_type::instance());

			reg.des_engine_ptr()->system_initialization_event_source().connect(
				::dcs::functional::bind(
					&self_type::process_sys_init,
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

			reg.des_engine_ptr()->system_initialization_event_source().disconnect(
				::dcs::functional::bind(
					&self_type::process_sys_init,
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

			reg.des_engine_ptr()->schedule_event(
				ptr_control_evt_src_,
				reg.des_engine_ptr()->simulated_time() + ts_
			);
		}

		do_schedule_control();
	}

	//@} Event Triggers


	//@{ Event Handlers


	private: void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		if (this->enabled())
		{
			schedule_control();
		}

		do_process_sys_init(evt, ctx);
	}


	private: void process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing CONTROL (Clock: " << ctx.simulated_time() << ")");//XXX

		if (this->enabled())
		{
			schedule_control();
		}

//		do_process_control(evt, ctx);
		control();

		DCS_DEBUG_TRACE("(" << this << ") END Processing CONTROL (Clock: " << ctx.simulated_time() << ")");//XXX
	}


	//@} Event Handlers

	//@{ Interface Member Functions

	protected: virtual void do_machine(physical_machine_pointer const& ptr_mach)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ptr_mach );

		// empty
	}


	protected: virtual void do_process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
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


	private: virtual void do_control() = 0;


//	private: virtual void do_process_control(des_event_type const& evt, des_engine_context_type& ctx) = 0;

	//@} Interface Member Functions


	private: physical_machine_pointer ptr_mach_;
	private: real_type ts_;
//	private: bool passive_;
	private: des_event_source_pointer ptr_control_evt_src_;
};

template <typename TraitsT>
const ::std::string base_physical_machine_controller<TraitsT>::control_event_source_name("Control Machine");

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_BASE_PHYSICAL_MACHINE_CONTROLLER_HPP
