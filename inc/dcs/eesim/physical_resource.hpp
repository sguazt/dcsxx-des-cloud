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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_EESIM_PHYSICAL_RESOURCE_HPP
#define DCS_EESIM_PHYSICAL_RESOURCE_HPP


#include <dcs/perfeval/energy/base_model.hpp>
#include <dcs/perfeval/energy/constant_model.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/physical_resource_view.hpp>
#include <dcs/exception.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <iosfwd>
#include <stdexcept>
#include <string>


namespace dcs { namespace eesim {

template <typename TraitsT>
class physical_resource: public physical_resource_view<TraitsT>
{
	private: typedef physical_resource_view<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	//public: typedef ::dcs::perfeval::energy::any_model<real_type> energy_model_type;
	public: typedef ::dcs::perfeval::energy::base_model<real_type> energy_model_type;
	public: typedef ::dcs::shared_ptr<energy_model_type> energy_model_pointer;


	/// Default constructor
	public: physical_resource()
	: base_type(),
	  name_("Unnamed Resource"),
	  ptr_energy_(new ::dcs::perfeval::energy::constant_model<real_type>(0))
	{
	}


	public: physical_resource(::std::string const& name,
							  physical_resource_category category,
							  real_type capacity,
							  real_type threshold = real_type(1))
		: base_type(category,capacity,threshold),
		  name_(name),
		  ptr_energy_(new ::dcs::perfeval::energy::constant_model<real_type>(0))
	{
	}


	public: physical_resource(::std::string const& name,
							  physical_resource_category category,
							  real_type capacity,
							  real_type threshold,
							   energy_model_pointer const& ptr_energy_model)
		: base_type(category,capacity,threshold),
		  name_(name),
		  ptr_energy_(ptr_energy_model)
	{
	}


	/// Copy constructor.
	private: physical_resource(physical_resource const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: physical_resource& operator=(physical_resource const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
	}


	public: void name(::std::string const& name)
	{
		name_ = name;
	}


	public: ::std::string const& name() const
	{
		return name_;
	}


	public: physical_resource_category category() const
	{
		return base_type::category();
	}


	public: real_type capacity() const
	{
		return base_type::capacity();
	}


	public: real_type utilization_threshold() const
	{
		return base_type::utilization_threshold();
	}


	public: void category(physical_resource_category category)
	{
		base_type::category(category);
	}


	public: void capacity(real_type c)
	{
		base_type::capacity(c);
	}


	public: void utilization_threshold(real_type t)
	{
		base_type::utilization_threshold(t);
	}


//	public: template <typename EnergyModelT>
//		void energy_model(EnergyModelT const& value)
//	{
//		energy_ = ::dcs::perfeval::energy::make_any_model(value);
//	}
//
//
//	public: real_type consumed_energy(real_type u) const
//	{
//		return energy_.consumed_energy(u);
//	}


	public: void energy_model(energy_model_pointer const& ptr_model)
	{
		ptr_energy_ = ptr_model;
	}


	public: real_type consumed_energy(real_type u) const
	{
		return ptr_energy_->consumed_energy(u);
	}


	public: energy_model_type& energy_model()
	{
		return *ptr_energy_;
	}


	public: energy_model_type const& energy_model() const
	{
		return *ptr_energy_;
	}


	private: ::std::string name_;
	private: energy_model_pointer ptr_energy_;
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
			  << ", Max Utilization: " << (resource.utilization_threshold()*100) << "%"
			  << ">";
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_PHYSICAL_RESOURCE_HPP
