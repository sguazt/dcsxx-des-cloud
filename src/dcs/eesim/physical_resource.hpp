/**
 * \file dcs/eesim/physical_resource.hpp
 *
 * \brief A physical resource.
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

#ifndef DCS_EESIM_PHYSICAL_RESOURCE_HPP
#define DCS_EESIM_PHYSICAL_RESOURCE_HPP


#include <dcs/perfeval/energy/any_model.hpp>
#include <dcs/perfeval/energy/constant_model.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <iostream>
#include <string>

namespace dcs { namespace eesim {

template <typename TraitsT>
class physical_resource
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef ::dcs::perfeval::energy::any_model<real_type> energy_model_type;


	public: physical_resource(::std::string const& name, physical_resource_category category, real_type capacity)
		: name_(name),
		  category_(category),
		  capacity_(capacity),
		  energy_(::dcs::perfeval::energy::make_any_model(::dcs::perfeval::energy::constant_model<real_type>(0)))
	{
		// empty
	}


	public: template <typename EnergyModelT>
		physical_resource(::std::string const& name, physical_resource_category category, real_type capacity, EnergyModelT const& energy_model)
		: name_(name),
		  category_(category),
		  capacity_(capacity),
		  energy_(::dcs::perfeval::energy::make_any_model(energy_model))
	{
		// empty
	}


	public: ::std::string const& name() const
	{
		return name_;
	}


	public: physical_resource_category category() const
	{
		return category_;
	}


	public: real_type capacity() const
	{
		return capacity_;
	}


	public: template <typename EnergyModelT>
		void energy_model(EnergyModelT const& value)
	{
		energy_ = ::dcs::perfeval::energy::make_any_model(value);
	}


	public: real_type consumed_energy(real_type u) const
	{
		return energy_.consumed_energy(u);
	}


//	public: energy_model_type& energy_model()
//	{
//		return energy_;
//	}


//	public: energy_model_type const& energy_model() const
//	{
//		return energy_;
//	}


	private: ::std::string name_;
	private: physical_resource_category category_;
	private: real_type capacity_;
	private: energy_model_type energy_;
};


template <
	typename CharT,
	typename CharTraitsT,
	typename RealT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, physical_resource<RealT> const& resource)
{
	return os << "<"
			  << "Name: " << resource.name()
			  << ", Category: " << resource.category()
			  << ", Capacity: " << resource.capacity()
			  << ">";
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_PHYSICAL_RESOURCE_HPP
