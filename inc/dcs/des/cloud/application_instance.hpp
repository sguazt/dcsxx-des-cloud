/**
 * \file dcs/eesim/application_instance.hpp
 *
 * \brief Class representing an instance of an application.
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

#ifndef DCS_EESIM_APPLICATION_INSTANCE_HPP
#define DCS_EESIM_APPLICATION_INSTANCE_HPP


#include <dcs/eesim/base_application_controller.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/exception.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <stdexcept>


namespace dcs { namespace eesim {

template <typename TraitsT>
class application_instance
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef multi_tier_application<traits_type> application_type;
	public: typedef ::dcs::shared_ptr<application_type> application_pointer;
	public: typedef base_application_controller<traits_type> application_controller_type;
	public: typedef ::dcs::shared_ptr<application_controller_type> application_controller_pointer;


	public: application_instance(application_pointer const& ptr_app,
								 application_controller_pointer const& ptr_app_ctrl,
								 real_type start_time,
								 real_type run_time)
	: ptr_app_(ptr_app),
	  ptr_app_ctrl_(ptr_app_ctrl),
	  start_time_(start_time),
	  stop_time_(start_time+run_time)
	{
	}


	/// Copy constructor.
	private: application_instance(application_instance<traits_type> const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: application_instance<traits_type>& operator=(application_instance<traits_type> const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assignment not yet implemented." );
	}


	public: application_pointer application_ptr() const
	{
		return ptr_app_;
	}


	public: application_pointer application_ptr()
	{
		return ptr_app_;
	}


	public: application_type const& application() const
	{
		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_app_ );

		return *ptr_app_;
	}


	public: application_type& application()
	{
		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_app_ );

		return *ptr_app_;
	}


	public: application_controller_pointer application_controller_ptr() const
	{
		return ptr_app_ctrl_;
	}


	public: application_controller_pointer application_controller_ptr()
	{
		return ptr_app_ctrl_;
	}


	public: application_controller_type const& application_controller() const
	{
		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_app_ctrl_ );

		return *ptr_app_ctrl_;
	}


	public: application_controller_type& application_controller()
	{
		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_app_ctrl_ );

		return *ptr_app_ctrl_;
	}


	public: real_type start_time() const
	{
		return start_time_;
	}


	public: real_type stop_time() const
	{
		return stop_time_;
	}


	protected: void application(application_pointer const& ptr_app)
	{
		ptr_app_ = ptr_app;
	}


	protected: void application_controller(application_controller_pointer const& ptr_app_ctrl)
	{
		ptr_app_ctrl_ = ptr_app_ctrl;
	}


	protected: void start_time(real_type time)
	{
		start_time_ = time;
	}


	protected: void stop_time(real_type time)
	{
		stop_time_ = time;
	}


	private: application_pointer ptr_app_;
	private: application_controller_pointer ptr_app_ctrl_;
	private: real_type start_time_;
	private: real_type stop_time_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_APPLICATION_INSTANCE_HPP
