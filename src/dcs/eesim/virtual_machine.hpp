/**
 * \file dcs/eesim/virtual_machine.hpp
 *
 * \brief Model for virtual machines.
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

#ifndef DCS_EESIM_VIRTUAL_MACHINE_HPP
#define DCS_EESIM_VIRTUAL_MACHINE_HPP


#include <dcs/math/stats/function/rand.hpp>
#include <dcs/eesim/power_status.hpp>
#include <dcs/memory.hpp>
#include <string>


namespace dcs { namespace eesim {

/**
 * \brief Model for virtual machines.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class virtual_machine
{
	public: explicit virtual_machine(std::string const& name="")
		: name_(name),
		  power_status_(powered_off_power_status)
	{
	}


//	public: void guest_system(::dcs::shared_ptr<virtual_guest_system> const& ptr_guest)
//	{
//		app_ = make_any_application(ptr_app)
//	}


	public: void power_on()
	{
		power_status_ = powered_on_power_status;
	}


	public: void power_off()
	{
		power_status_ = powered_off_power_status;
	}


	public: void suspend()
	{
		power_status_ = suspended_power_status;
	}


	public: power_status power_state() const
	{
		return power_status_;
	}


	private: ::std::string name_;
//	private: queue_model_type queue_;
	private: power_status power_status_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_VIRTUAL_MACHINE_HPP
