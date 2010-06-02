/**
 * \file dcs/eesim/virtual_machine_monitor.hpp
 *
 * \brief Virtual machine monitor.
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

#ifndef DCS_EESIM_VIRTUAL_MACHINE_MONITOR_HPP
#define DCS_EESIM_VIRTUAL_MACHINE_MONITOR_HPP


#include <dcs/assert.hpp>
#include <dcs/eesim/virtual_machine.hpp>
//#include <map>
#include <stdexcept>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class virtual_machine_monitor
{
	//public: typedef any_queue_model queue_model_type;
	//public: typedef virtual_machine<queue_model_type> virtual_machine_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef virtual_machine<traits_type> virtual_machine_type;
	public: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	//private: typedef ::std::map<uint_type,virtual_machine_pointer> virtual_machine_container;
	private: typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;


	public: virtual_machine_monitor()
		: vms_counter_(0)
	{
	}


	public: uint_type add_virtual_machine(virtual_machine_pointer const& ptr_vm)
	{
		//++vms_counter_;

		vms_.push_back(ptr_vm);
		//vms_.insert(::std::make_pair(vms_counter_, ptr_vm));
		//vm_id_names.insert(::std::make_pair(vms_counter_, ptr_vm->name()));

		//return vms_counter_;
		return (vms_.size()-1);
	}


	public: void overhead(real_type value)
	{
		overhead_ = value;
	}


	public: real_type overhead() const
	{
		return overhead_;
	}


	public: void power_on(uint_type vm_id) // throws...
	{
		// preconditions
		DCS_ASSERT(
			vm_id < vms_.size(),
			throw ::std::runtime_error("Unknown VM")
		);

		vms_[vm_id]->power_on();
	}


	public: void power_off(uint_type vm_id) // throws...
	{
		// preconditions
		DCS_ASSERT(
			vm_id < vms_.size(),
			throw ::std::runtime_error("Unknown VM")
		);

		vms_[vm_id]->power_off();
	}


	public: void suspend(uint_type vm_id) // throws...
	{
		// preconditions
		DCS_ASSERT(
			vm_id < vms_.size(),
			throw ::std::runtime_error("Unknown VM")
		);

		vms_[vm_id]->suspend();
	}


	private: uint_type vms_counter_;
	private: virtual_machine_container vms_;
	//private: uint_type_name_container vm_id_names_;
	/// The performance overhead associated to virtualization
	private: real_type overhead_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_VIRTUAL_MACHINE_MONITOR_HPP
