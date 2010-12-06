/**
 * \file dcs/eesim/base_application_controller.hpp
 *
 * \brief Base class for application controllers.
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
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_EESIM_BASE_APPLICATION_CONTROLLER_HPP
#define DCS_EESIM_BASE_APPLICATION_CONTROLLER_HPP


#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/functional/bind.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <limits>


namespace dcs { namespace eesim {

template <typename TraitsT>
class base_application_controller
{
	private: typedef base_application_controller<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef multi_tier_application<traits_type> application_type;
	public: typedef ::dcs::shared_ptr<application_type> application_pointer;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	public: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	private: typedef registry<traits_type> registry_type;


	/// Default constructor.
	protected: base_application_controller()
	: ptr_app_(),
	  ts_(::std::numeric_limits<real_type>::infinity()),
	  ptr_control_evt_src_(new des_event_source_type())
	{
		init();
	}


	/// A constructor.
	protected: base_application_controller(application_pointer const& ptr_app, real_type ts)
	: ptr_app_(ptr_app),
	  ts_(ts),
	  ptr_control_evt_src_(new des_event_source_type())
	{
		init();
	}


	/// Copy constructor.
	protected: base_application_controller(base_application_controller const& that)
	{
		ptr_app_ = that.ptr_app_;
		ts_ = that.ts_;
		ptr_control_evt_src_ = ::dcs::make_shared<des_event_source_type>(*that.ptr_control_evt_src_);
	}


	/// Copy assignment.
	protected: base_application_controller& operator=(base_application_controller const& rhs)
	{
		if (this != &rhs)
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
			ptr_app_ = rhs.ptr_app_;
			ts_ = rhs.ts_;
			ptr_control_evt_src_ = ::dcs::make_shared<des_event_source_type>(*(rhs.ptr_control_evt_src_));
			//ptr_control_evt_src_ = des_event_source_pointer(new des_event_source_type>(*rhs.ptr_control_evt_src_));
		}

		return *this;
	}


	/// The destructor.
	public: virtual ~base_application_controller()
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
	}


	public:void controlled_application(application_pointer const& ptr_app)
	{
		ptr_app_ = ptr_app;
	}


	public: application_pointer controlled_application() const
	{
		return ptr_app_;
	}


	public: application_pointer controlled_application()
	{
		return ptr_app_;
	}


	public: des_event_source_type& contrl_event_source()
	{
		return *ptr_control_evt_src_;
	}


	public: des_event_source_type const& contrl_event_source() const
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


	public: des_event_source_type& control_event_source()
	{
		return *ptr_control_evt_src_;
	}


	public: des_event_source_type const& control_event_source() const
	{
		return *ptr_control_evt_src_;
	}


	protected: application_pointer application_ptr() const
	{
		return ptr_app_;
	}


	protected: application_pointer application_ptr()
	{
		return ptr_app_;
	}


	private: void init()
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

		reg.des_engine_ptr()->system_initialization_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_sys_init,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		schedule_control();

		do_process_sys_init(evt, ctx);
	}


	private: void process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		do_process_control(evt, ctx);
	}


	private: void schedule_control()
	{
		registry_type& reg(registry_type::instance());

		reg.des_engine_ptr()->schedule_event(
			ptr_control_evt_src_,
			reg.des_engine_ptr()->simulated_time() + ts_
		);

		do_schedule_control();
	}


	private: virtual void do_process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
	}


	private: virtual void do_process_control(des_event_type const& evt, des_engine_context_type& ctx) = 0;


	private: virtual void do_schedule_control()
	{
	}


	private: application_pointer ptr_app_;
	private: real_type ts_;
	private: des_event_source_pointer ptr_control_evt_src_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_APPLICATION_CONTROLLER_HPP
