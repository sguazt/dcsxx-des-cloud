/**
 * \file dcs/des/cloud/dummy_physical_machine_controller.hpp
 *
 * \brief Dummy Physical Machine Controller.
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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_CLOUD_DUMMY_PHYSICAL_MACHINE_CONTROLLER_HPP
#define DCS_DES_CLOUD_DUMMY_PHYSICAL_MACHINE_CONTROLLER_HPP


#include <dcs/des/engine_traits.hpp>
#include <dcs/des/cloud/base_physical_machine_controller.hpp>
#include <dcs/macro.hpp>


namespace dcs { namespace des { namespace cloud {

/**
 * \brief Dummy physical machine controller.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename TraitsT>
class dummy_physical_machine_controller: public base_physical_machine_controller<TraitsT>
{
	private: typedef base_physical_machine_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename base_type::physical_machine_pointer physical_machine_pointer;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;


	public: dummy_physical_machine_controller()
	: base_type()
	{
	}


	public: dummy_physical_machine_controller(physical_machine_pointer const& ptr_mach)
	: base_type(ptr_mach)
	{
	}


    private: void do_control()
	{
		// empty
	}
};

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_DUMMY_PHYSICAL_MACHINE_CONTROLLER_HPP
