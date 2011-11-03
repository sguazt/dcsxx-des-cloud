/**
 * \file dcs/eesim/any_physical_machine.hpp
 *
 * \brief Generic (type-erased) class for physical machines.
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

#ifndef DCS_EESIM_ANY_PHYSICAL_MACHINE_HPP
#define DCS_EESIM_ANY_PHYSICAL_MACHINE_HPP


#include <dcs/perfeval/energy/any_model.hpp>
#include <dcs/eesim/any_virtual_machine_monitor.hpp>
#include <dcs/eesim/base_physical_machine.hpp>
#include <dcs/eesim/physical_machine_adaptor.hpp>
#include <dcs/eesim/physical_resource.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/power_status.hpp>
#include <dcs/exception.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <dcs/util/holder.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class any_physical_machine
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef physical_resource<traits_type> physical_resource_type;
	private: typedef base_physical_machine<traits_type> base_physical_machine_type;


	/// Default constructor.
	public: any_physical_machine()
	{
	}


	public: template <typename PhysicalMachineT>
		explicit any_physical_machine(PhysicalMachineT const& machine)
		: ptr_machine_(new physical_machine_adaptor<PhysicalMachineT>(machine))
	{
	}


	public: template <typename PhysicalMachineT>
		explicit any_physical_machine(::dcs::util::holder<PhysicalMachineT> const& wrap_machine)
		: ptr_machine_(new physical_machine_adaptor<PhysicalMachineT>(wrap_machine.get()))
	{
	}


	/// Copy constructor.
	private: any_physical_machine(any_physical_machine const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: any_physical_machine& operator=(any_physical_machine const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
	}


	public: template <typename PhysicalMachineT>
		void physical_machine(PhysicalMachineT machine)
	{
		ptr_machine_ = new physical_machine_adaptor<PhysicalMachineT>(machine);
	}


	public: ::std::string const& name() const
	{
		return ptr_machine_->name();
	}


	public: ::std::vector<physical_resource_type> resources(physical_resource_category category) const
	{
		return ptr_machine_->resources(category);
	}


	public: ::std::vector<physical_resource_type> resources() const
	{
		return ptr_machine_->resources();
	}


	public: ::dcs::perfeval::energy::any_model<real_type> const& energy_model() const
	{
		return ptr_machine_->energy_model();
	}


	public: ::dcs::perfeval::energy::any_model<real_type>& energy_model()
	{
		return ptr_machine_->energy_model();
	}


	public: any_virtual_machine_monitor<traits_type> const& virtual_machine_monitor() const
	{
		return ptr_machine_->virtual_machine_monitor();
	}


	public: any_virtual_machine_monitor<traits_type>& virtual_machine_monitor()
	{
		return ptr_machine_->virtual_machine_monitor();
	}


//	public: des_engine_pointer const& des_engine() const
//	{
//		return ptr_machine_->des_engine();
//	}


//	public: des_engine_pointer& des_engine()
//	{
//		return ptr_machine_->des_engine();
//	}


	public: real_type cost() const
	{
		return ptr_machine_->cost();
	}


	public: power_status power_state() const
	{
		return ptr_machine_->power_state();
	}


	public: void power_on()
	{
		ptr_machine_->power_on();
	}


	public: void power_off()
	{
		ptr_machine_->power_off();
	}


	public: void suspend()
	{
		ptr_machine_->suspend();
	}


	private: ::dcs::shared_ptr<base_physical_machine_type> ptr_machine_;
};


template <
	typename PhysicalMachineT,
	typename PhysicalMachineTraitsT=typename ::dcs::type_traits::remove_reference<PhysicalMachineT>::type
>
struct make_any_physical_machine_type
{
	typedef any_physical_machine<typename PhysicalMachineTraitsT::traits_type> type;
};


namespace detail {

template <
	typename PhysicalMachineT,
	typename PhysicalMachineTraitsT=typename ::dcs::type_traits::remove_reference<PhysicalMachineT>::type
>
struct make_any_physical_machine_impl;


template <typename PhysicalMachineT, typename PhysicalMachineTraitsT>
struct make_any_physical_machine_impl
{
	typedef typename make_any_physical_machine_type<PhysicalMachineT,PhysicalMachineTraitsT>::type any_physical_machine_type;
	static any_physical_machine_type apply(PhysicalMachineT& machine)
	{
		return any_physical_machine_type(machine);
	}
};


template <typename PhysicalMachineT, typename PhysicalMachineTraitsT>
struct make_any_physical_machine_impl<PhysicalMachineT&,PhysicalMachineTraitsT>
{
	typedef typename make_any_physical_machine_type<PhysicalMachineT,PhysicalMachineTraitsT>::type any_physical_machine_type;
	static any_physical_machine_type apply(PhysicalMachineT& machine)
	{
		::dcs::util::holder<PhysicalMachineT&> wrap_machine(machine);
		return any_physical_machine_type(wrap_machine);
	}
};

} // Namespace detail


template <typename PhysicalMachineT>
typename make_any_physical_machine_type<PhysicalMachineT>::type make_any_physical_machine(PhysicalMachineT machine)
{
	return detail::make_any_physical_machine_impl<PhysicalMachineT>::apply(machine);
}



template <
	typename CharT,
	typename CharTraitsT,
	typename TraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, any_physical_machine<TraitsT> const& machine)
{
	typedef any_physical_machine<TraitsT> machine_type;
	typedef typename machine_type::physical_resource_type resource_type;

	os << "<Name: " << machine.name()
	   << ", Resources: {";

	::std::vector<resource_type> resources = machine.resources();
	typename ::std::vector<resource_type>::const_iterator it_begin = resources.begin();
	typename ::std::vector<resource_type>::const_iterator it_end = resources.end();
	for (
		typename ::std::vector<resource_type>::const_iterator it = it_begin;
		it != it_end;
		++it
	) {
		if (it != it_begin)
		{
			os << ", ";
		}
		os << *it;
	}

	os << "}"
	   << ", Power status: " << machine.power_state()
	   << ">";

	return os;
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_ANY_PHYSICAL_MACHINE_HPP
