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


#include <dcs/eesim/virtual_machine_monitor.hpp>
#include <dcs/eesim/physical_resource.hpp>
#include <dcs/eesim/power_status.hpp>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>


namespace dcs { namespace eesim {

//NOTE: for each resource category it is possible to associate at most one resource object.
//      So if you multiple CPUs, you have to create only one CPU resource object with the aggregated capacity.
//      TODO: maybe we can add a "multiplicity" attribute to a resource object.
template <typename TraitsT>
class physical_machine
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::physical_machine_identifier_type identifier_type;
	public: typedef physical_resource<traits_type> resource_type;
	public: typedef ::dcs::shared_ptr<resource_type> resource_pointer;
	private: typedef ::std::map<
						physical_resource_category,
						resource_pointer
					> resource_container;
	//private: typedef any_virtual_machine_monitor<traits_type> virtual_machine_monitor_type;
	public: typedef virtual_machine_monitor<traits_type> vmm_type;
	public: typedef ::dcs::shared_ptr<vmm_type> vmm_pointer;
	private: typedef typename resource_container::iterator resource_iterator;
	private: typedef typename resource_container::const_iterator resource_const_iterator;


	public: explicit physical_machine(::std::string const& name = "Unnamed Physical Machine")
		: id_(traits_type::invalid_physical_machine_id),
		  name_(name),
		  power_status_(powered_off_power_status),
		  cost_(0)
	{
	}


	public: void name(::std::string const& s)
	{
		name_ = s;
	}


	public: ::std::string const& name() const
	{
		return name_;
	}


	public: void id(identifier_type x)
	{
		id_ = x;
	}


	public: identifier_type id() const
	{
		return id_;
	}


	public: void add_resource(resource_pointer const& ptr_resource)
	{
		//resources_.insert(::std::make_pair(ptr_resource->category(), ptr_resource));
		resources_[ptr_resource->category()] = ptr_resource;
	}


//	public: ::std::vector<physical_resource_pointer> resources(physical_resource_category category) const
//	{
//		::std::pair<resource_const_iterator,resource_const_iterator> it_pair;
//		it_pair = resources_.equal_range(category);
//
//		::std::vector<resource_pointer> resources;
//
//		while (it_pair.first != it_pair.second)
//		{
//			resources.push_back(it_pair.first->second);
//			++(it_pair.first);
//		}
//
//		return resources;
//	}


	public: resource_pointer const& resource(physical_resource_category category) const
	{
		typename resource_container::const_iterator it;
		it = resources_.find(category);

		// safety check
		DCS_ASSERT(
			it != resources_.end(),
			throw ::std::logic_error("[dcs::eesim::physical_machine::resource] Resource not found on this machine.")
		);

		return it->second;
	}


	public: ::std::vector<resource_pointer> resources() const
	{
		::std::vector<resource_pointer> resources;

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


//	public: template <typename VirtualMachineMonitorT>
//		void virtual_machine_monitor(VirtualMachineMonitorT const& vmm)
//	{
//		vmm_ = make_any_virtual_machine_monitor(vmm);
//	}


//	public: virtual_machine_monitor_type& virtual_machine_monitor()
//	{
//		return vmm_;
//	}


//	public: virtual_machine_monitor_type const& virtual_machine_monitor() const
//	{
//		return vmm_;
//	}


	public: void vmm(vmm_pointer const& ptr_vmm)
	{
		ptr_vmm_ = ptr_vmm;
//		ptr_vmm_->hosting_machine(this);
	}


	public: vmm_pointer vmm() const
	{
		return ptr_vmm_;
	}


	public: vmm_pointer vmm_ptr()
	{
		return ptr_vmm_;
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


	public: void resume()
	{
		power_status_ = powered_on_power_status;
	}


	public: power_status power_state() const
	{
		return power_status_;
	}


	//FIXME: assume that total energy is additive
	public: real_type consumed_energy(real_type u) const
	{
		resource_iterator it_end = resources_.end();

		real_type energy = 0;

		for (resource_iterator it = resources_.begin(); it != it_end; ++it)
		{
			energy += (*it)->consumed_energy(u);
		}

		return energy;
	}


	/// The unique machine identifier.
	private: identifier_type id_;
	/// The mnemonic name for this machine.
	private: ::std::string name_;
	/// Tell if this machine is powered on.
	private: power_status power_status_;
	/// The cost for using this machine.
	private: real_type cost_;
	/// The set of physical resources this machine is equipped.
	private: resource_container resources_;
	/// The virtual machine monitor.
	private: vmm_pointer ptr_vmm_;
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
	typedef typename machine_type::resource_pointer resource_pointer;
	typedef ::std::vector<resource_pointer> resource_container;
	typedef typename resource_container::const_iterator resource_iterator;

    os << "<Name: " << machine.name()
	   << ", Resources: {";

	resource_container resources = machine.resources();
	resource_iterator it_begin = resources.begin();
	resource_iterator it_end = resources.end();
	for (resource_iterator it = it_begin; it != it_end; ++it)
	{
		if (it != it_begin)
		{
			os << ", ";
		}
		os << **it;
	}

	os << "}"
	   << ", Power status: " << machine.power_state()
	   << ">";

	return os;
}


}} // Namespace dcs::esim


#endif // DCS_EESIM_PHYSICAL_MACHINE_HPP
