/**
 * \file dcs/eesim/physical_machine_adaptor.hpp
 *
 * \brief Adaptor class for physical machines.
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

#ifndef DCS_EESIM_PHYSICAL_MACHINE_ADAPTOR_HPP
#define DCS_EESIM_PHYSICAL_MACHINE_ADAPTOR_HPP


#include <dcs/perfeval/energy/any_model.hpp>
#include <dcs/eesim/base_physical_machine.hpp>
#include <dcs/eesim/physical_resource.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/exception.hpp>
#include <dcs/macro.hpp>
#include <dcs/type_traits/add_reference.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/remove_reference.hpp>
#include <stdexcept>
#include <vector>


namespace dcs { namespace eesim {

/**
 * \brief Adaptor class for physical machines.
 *
 * \tparam PhysicalMachineT The adaptee physical machine class type.
 * \tparam PhysicalMachineTraitsT Type traits for the adaptee physical machine
 *  class type.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <
	typename PhysicalMachineT,
	typename PhysicalMachineTraitsT=typename ::dcs::type_traits::remove_reference<PhysicalMachineT>::type
>
class physical_machine_adaptor: public base_physical_machine<
										typename PhysicalMachineTraitsT::traits_type
								>
{
	public: typedef PhysicalMachineT physical_machine_type;
	public: typedef typename PhysicalMachineTraitsT::traits_type traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef ::dcs::perfeval::energy::any_model<real_type> energy_model_type;
	public: typedef physical_resource<traits_type> physical_resource_type;
	private: typedef typename ::dcs::type_traits::add_reference<physical_machine_type>::type machine_reference;
	private: typedef typename ::dcs::type_traits::add_reference<typename ::dcs::type_traits::add_const<physical_machine_type>::type>::type machine_const_reference;
	private: typedef typename PhysicalMachineTraitsT::energy_model_type adaptee_energy_model_type;
	private: typedef typename PhysicalMachineTraitsT::virtual_machine_monitor_type adaptee_vmm_type;


	public: physical_machine_adaptor(machine_const_reference adaptee_machine)
		: machine_(adaptee_machine),
		  energy_model_(
			::dcs::perfeval::energy::make_any_model<adaptee_energy_model_type&>(
				machine_.energy_model()
			)
		  ),
		  vmm_(
			make_any_virtual_machine_monitor<adaptee_vmm_type&>(
				machine_.virtual_machine_monitor()
			)
		  )
	{
	}


	/// Copy constructor.
	private: physical_machine_adaptor(physical_machine_adaptor const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: physical_machine_adaptor& operator=(physical_machine_adaptor const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
	}


	public: ::std::string const& name() const
	{
		return machine_.name();
	}


	public: ::std::vector<physical_resource_type> resources(physical_resource_category category) const
	{
		return machine_.resources(category);
	}


	public: ::std::vector<physical_resource_type> resources() const
	{
		return machine_.resources();
	}


	public: ::dcs::perfeval::energy::any_model<real_type> const& energy_model() const
	{
		return energy_model_;
	}


	public: ::dcs::perfeval::energy::any_model<real_type>& energy_model()
	{
		return energy_model_;
	}


	public: any_virtual_machine_monitor<traits_type> const& virtual_machine_monitor() const
	{
		return vmm_;
	}


	public: any_virtual_machine_monitor<traits_type>& virtual_machine_monitor()
	{
		return vmm_;
	}


	public: real_type cost() const
	{
		return machine_.cost();
	}


	public: power_status power_state() const
	{
		return machine_.power_state();
	}


	public: void power_on()
	{
		machine_.power_on();
	}


	public: void power_off()
	{
		machine_.power_off();
	}


	public: void suspend()
	{
		machine_.suspend();
	}


	private: physical_machine_type machine_;
	private: ::dcs::perfeval::energy::any_model<real_type> energy_model_;
	private: any_virtual_machine_monitor<traits_type> vmm_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_PHYSICAL_MACHINE_ADAPTOR_HPP
