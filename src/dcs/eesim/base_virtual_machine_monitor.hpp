/**
 * \file dcs/eesim/base_virtual_machine_monitor.hpp
 *
 * \brief Base class for physical machines.
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

#ifndef DCS_EESIM_BASE_VIRTUAL_MACHINE_MONITOR_HPP
#define DCS_EESIM_BASE_VIRTUAL_MACHINE_MONITOR_HPP


#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/memory.hpp>
#include <iostream>


namespace dcs { namespace eesim {

template <typename TraitsT>
struct base_virtual_machine_monitor
{
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef typename traits_type::uint_type uint_type;
	typedef virtual_machine<traits_type> virtual_machine_type;
	typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;


	virtual ~base_virtual_machine_monitor() { }

	virtual uint_type add_virtual_machine(virtual_machine_pointer const& ptr_vm) = 0;

	virtual real_type overhead() const = 0;

	virtual void power_on(uint_type vm_id) = 0;

	virtual void power_off(uint_type vm_id) = 0;

	virtual void suspend(uint_type vm_id) = 0;
};


///\todo
template <
    typename CharT,
    typename CharTraitsT,
    typename TraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, base_virtual_machine_monitor<TraitsT> const& vmm)
{
	//TODO

	return os;
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_VIRTUAL_MACHINE_MONITOR_HPP
