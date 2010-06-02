/**
 * \file dcs/eesim/base_physical_machine.hpp
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

#ifndef DCS_EESIM_BASE_PHYSICAL_MACHINE_HPP
#define DCS_EESIM_BASE_PHYSICAL_MACHINE_HPP


#include <dcs/perfeval/energy/any_model.hpp>
#include <dcs/eesim/physical_resource.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/power_status.hpp>
#include <dcs/eesim/any_virtual_machine_monitor.hpp>
#include <iostream>
#include <string>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
struct base_physical_machine
{
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef physical_resource<traits_type> physical_resource_type;


	virtual ~base_physical_machine() { }

	virtual ::std::string const& name() const = 0;

	virtual ::std::vector<physical_resource_type> resources(physical_resource_category category) const = 0;

	virtual ::std::vector<physical_resource_type> resources() const = 0;

	virtual ::dcs::perfeval::energy::any_model<real_type> const& energy_model() const = 0;

	virtual ::dcs::perfeval::energy::any_model<real_type>& energy_model() = 0;

	virtual any_virtual_machine_monitor<traits_type> const& virtual_machine_monitor() const = 0;

	virtual any_virtual_machine_monitor<traits_type>& virtual_machine_monitor() = 0;

	virtual real_type cost() const = 0;

	virtual power_status power_state() const = 0;

	virtual void power_on() = 0;

	virtual void power_off() = 0;

	virtual void suspend() = 0;
};


template <
    typename CharT,
    typename CharTraitsT,
    typename TraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, base_physical_machine<TraitsT> const& machine)
{
	typedef base_physical_machine<TraitsT> machine_type;
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


#endif // DCS_EESIM_BASE_PHYSICAL_MACHINE_HPP
