/**
 * \file dcs/eesim/physical_machine.hpp
 *
 * \brief Physical machine.
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

#ifndef DCS_EESIM_PHYSICAL_MACHINE_HPP
#define DCS_EESIM_PHYSICAL_MACHINE_HPP


#include <dcs/eesim/any_virtual_machine_monitor.hpp>
#include <dcs/eesim/physical_resource.hpp>
#include <dcs/eesim/power_status.hpp>
#include <iostream>
#include <map>
#include <string>
#include <utility>


namespace dcs { namespace eesim {

template <typename TraitsT>
class physical_machine
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef physical_resource<traits_type> physical_resource_type;
	private: typedef ::std::multimap<
						physical_resource_category,
						physical_resource_type
					> resource_container;
	private: typedef any_virtual_machine_monitor<traits_type> virtual_machine_monitor_type;
	private: typedef typename resource_container::iterator resource_iterator;
	private: typedef typename resource_container::const_iterator resource_const_iterator;


	public: physical_machine()
		: power_status_(powered_off_power_status)
	{
	}


	public: void name(::std::string const& name)
	{
		name_ = name;
	}


	public: ::std::string const& name() const
	{
		return name_;
	}


	public: void add_resource(physical_resource_type const& value)
	{
		resources_.insert(::std::make_pair(value.category(), value));
	}


	public: ::std::vector<physical_resource_type> resources(physical_resource_category category) const
	{
		::std::pair<resource_const_iterator,resource_const_iterator> it_pair;
		it_pair = resources_.equal_range(category);

		::std::vector<physical_resource_type> resources;

		while (it_pair.first != it_pair.second)
		{
			resources.push_back(it_pair.first->second);
			++(it_pair.first);
		}

		return resources;
	}


	public: ::std::vector<physical_resource_type> resources() const
	{
		::std::vector<physical_resource_type> resources;

		resource_const_iterator it_end = resources_.end();
		for (resource_const_iterator it = resources_.begin(); it != it_end; ++it)
		{
			resources.push_back(it->second);
		}

		return resources;
	}


	public: void cost(real_type value)
	{
		cost_ = value;
	}


	public: real_type cost() const
	{
		return cost_;
	}


	public: template <typename VirtualMachineMonitorT>
		void virtual_machine_monitor(VirtualMachineMonitorT const& vmm)
	{
		vmm_ = make_any_virtual_machine_monitor(vmm);
	}


	public: virtual_machine_monitor_type& virtual_machine_monitor()
	{
		return vmm_;
	}


	public: virtual_machine_monitor_type const& virtual_machine_monitor() const
	{
		return vmm_;
	}


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


	/// The mnemonic name for this machine.
	private: ::std::string name_;
	/// The set of physical resources this machine is equipped.
	private: resource_container resources_;
	/// The virtual machine monitor.
	private: virtual_machine_monitor_type vmm_;
	/// The cost for using this machine.
	private: real_type cost_;
	/// Tell if this machine is powered on.
	private: power_status power_status_;
};


// Output stream operator
template <
	typename CharT,
	typename CharTraitsT,
	typename TraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, physical_machine<TraitsT> const& machine)
{
	typedef physical_machine<TraitsT> machine_type;
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


}} // Namespace dcs::esim


#endif // DCS_EESIM_PHYSICAL_MACHINE_HPP
