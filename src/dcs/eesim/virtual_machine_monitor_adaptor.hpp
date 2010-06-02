/**
 * \file dcs/eesim/virtual_machine_monitor_adaptor.hpp
 *
 * \brief Adaptor class for virtual machine monitors.
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

#ifndef DCS_EESIM_VIRTUAL_MACHINE_MONITOR_ADAPTOR_HPP
#define DCS_EESIM_VIRTUAL_MACHINE_MONITOR_ADAPTOR_HPP


#include <dcs/eesim/base_virtual_machine_monitor.hpp>
#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/memory.hpp>
#include <dcs/type_traits/add_reference.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/remove_reference.hpp>


namespace dcs { namespace eesim {

/**
 * \brief Adaptor class for virtual machine monitors.
 *
 * \tparam VirtualMachineMonitorT The adaptee virtual machine monitor class
 *  type.
 * \tparam VirtualMachineMonitorTraitsT Type traits for the adaptee virtual
 *  machine monitor class type.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <
	typename VirtualMachineMonitorT,
	typename VirtualMachineMonitorTraitsT=typename ::dcs::type_traits::remove_reference<VirtualMachineMonitorT>::type
>
class virtual_machine_monitor_adaptor: public base_virtual_machine_monitor<
										typename VirtualMachineMonitorTraitsT::traits_type
								>
{
	public: typedef VirtualMachineMonitorT virtual_machine_monitor_type;
	public: typedef typename VirtualMachineMonitorTraitsT::traits_type traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename VirtualMachineMonitorTraitsT::virtual_machine_type virtual_machine_type; 
	public: typedef typename VirtualMachineMonitorTraitsT::virtual_machine_pointer virtual_machine_pointer; 
	private: typedef typename ::dcs::type_traits::add_reference<virtual_machine_monitor_type>::type machine_reference;
	private: typedef typename ::dcs::type_traits::add_reference<typename ::dcs::type_traits::add_const<virtual_machine_monitor_type>::type>::type machine_const_reference;


	public: virtual_machine_monitor_adaptor(machine_const_reference adaptee_vmm)
		: vmm_(adaptee_vmm)
	{
	}


	public: uint_type add_virtual_machine(virtual_machine_pointer const& ptr_vm)
	{
		return vmm_.add_virtual_machine(ptr_vm);
	}


	public: real_type overhead() const
	{
		return vmm_.overhead();
	}


	public: void power_on(uint_type vm_id)
	{
		vmm_.power_on(vm_id);
	}


	public: void power_off(uint_type vm_id)
	{
		vmm_.power_off(vm_id);
	}


	public: void suspend(uint_type vm_id)
	{
		vmm_.suspend(vm_id);
	}


	private: virtual_machine_monitor_type vmm_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_VIRTUAL_MACHINE_MONITOR_ADAPTOR_HPP
