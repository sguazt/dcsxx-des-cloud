/**
 * \file dcs/eesim/any_virtual_machine_monitor.hpp
 *
 * \brief Generic (type-erased) class for virtual machine monitors.
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

#ifndef DCS_EESIM_ANY_VIRTUAL_MACHINE_MONITOR_HPP
#define DCS_EESIM_ANY_VIRTUAL_MACHINE_MONITOR_HPP


#include <dcs/eesim/base_virtual_machine_monitor.hpp>
#include <dcs/eesim/virtual_machine_monitor_adaptor.hpp>
#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/memory.hpp>
#include <dcs/util/holder.hpp>
#include <iostream>


namespace dcs { namespace eesim {

template <typename TraitsT>
class any_virtual_machine_monitor
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef virtual_machine<traits_type> virtual_machine_type;
	public: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	private: typedef base_virtual_machine_monitor<traits_type> base_virtual_machine_monitor_type;


	public: any_virtual_machine_monitor()
	{
	}


	public: template <typename VirtualMachineMonitorT>
		any_virtual_machine_monitor(VirtualMachineMonitorT const& vmm)
		: ptr_vmm_(new virtual_machine_monitor_adaptor<VirtualMachineMonitorT>(vmm))
	{
	}


	public: template <typename VirtualMachineMonitorT>
		any_virtual_machine_monitor(::dcs::util::holder<VirtualMachineMonitorT> const& wrap_vmm)
		: ptr_vmm_(new virtual_machine_monitor_adaptor<VirtualMachineMonitorT>(wrap_vmm.get()))
	{
	}


	public: template <typename VirtualMachineMonitorT>
		void virtual_machine_monitor(VirtualMachineMonitorT vmm)
	{
		ptr_vmm_ = new virtual_machine_monitor_adaptor<VirtualMachineMonitorT>(vmm);
	}


//	public: des_engine_pointer const& des_engine() const
//	{
//		return ptr_vmm_->des_engine();
//	}


//	public: des_engine_pointer& des_engine()
//	{
//		return ptr_vmm_->des_engine();
//	}


	public: void create_virtual_machine(virtual_machine_pointer const& ptr_vm)
	{
		ptr_vmm_->create_virtual_machine(ptr_vm);
	}


	public: void destroy_virtual_machine(virtual_machine_pointer const& ptr_vm)
	{
		ptr_vmm_->destroy_virtual_machine(ptr_vm);
	}


	public: real_type overhead() const
	{
		return ptr_vmm_->overhead();
	}


	public: void power_on(virtual_machine_pointer const& ptr_vm)
	{
		ptr_vmm_->power_on(ptr_vm);
	}


	public: void power_off(virtual_machine_pointer const& ptr_vm)
	{
		ptr_vmm_->power_off(ptr_vm);
	}


	public: void suspend(virtual_machine_pointer const& ptr_vm)
	{
		ptr_vmm_->suspend(ptr_vm);
	}


	public: void resume(virtual_machine_pointer const& ptr_vm)
	{
		ptr_vmm_->resume(ptr_vm);
	}


	private: ::dcs::shared_ptr<base_virtual_machine_monitor_type> ptr_vmm_;
};


template <
	typename VirtualMachineMonitorT,
	typename VirtualMachineMonitorTraitsT=typename ::dcs::type_traits::remove_reference<VirtualMachineMonitorT>::type
>
struct make_any_virtual_machine_monitor_type
{
	typedef any_virtual_machine_monitor<
				typename VirtualMachineMonitorTraitsT::traits_type
			> type;
};


namespace detail {

template <
	typename VirtualMachineMonitorT,
	typename VirtualMachineMonitorTraitsT=typename ::dcs::type_traits::remove_reference<VirtualMachineMonitorT>::type
>
struct make_any_virtual_machine_monitor_impl;


template <typename VirtualMachineMonitorT, typename VirtualMachineMonitorTraitsT>
struct make_any_virtual_machine_monitor_impl
{
	typedef typename make_any_virtual_machine_monitor_type<VirtualMachineMonitorT,VirtualMachineMonitorTraitsT>::type any_virtual_machine_monitor_type;
	static any_virtual_machine_monitor_type apply(VirtualMachineMonitorT& vmm)
	{
		return any_virtual_machine_monitor_type(vmm);
	}
};


template <typename VirtualMachineMonitorT, typename VirtualMachineMonitorTraitsT>
struct make_any_virtual_machine_monitor_impl<VirtualMachineMonitorT&,VirtualMachineMonitorTraitsT>
{
	typedef typename make_any_virtual_machine_monitor_type<VirtualMachineMonitorT,VirtualMachineMonitorTraitsT>::type any_virtual_machine_monitor_type;
	static any_virtual_machine_monitor_type apply(VirtualMachineMonitorT& vmm)
	{
		::dcs::util::holder<VirtualMachineMonitorT&> wrap_vmm(vmm);
		return any_virtual_machine_monitor_type(wrap_vmm);
	}
};

} // Namespace detail


template <typename VirtualMachineMonitorT>
typename make_any_virtual_machine_monitor_type<VirtualMachineMonitorT>::type make_any_virtual_machine_monitor(VirtualMachineMonitorT machine)
{
	return detail::make_any_virtual_machine_monitor_impl<VirtualMachineMonitorT>::apply(machine);
}



/// \todo
template <
    typename CharT,
    typename CharTraitsT,
    typename TraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, any_virtual_machine_monitor<TraitsT> const& vmm)
{
	//TODO

	return os;
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_ANY_VIRTUAL_MACHINE_MONITOR_HPP
