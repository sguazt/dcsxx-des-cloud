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


#include <dcs/assert.hpp>
//#include <dcs/perfeval/energy/any_model.hpp>
#include <dcs/perfeval/energy/base_model.hpp>
#include <dcs/perfeval/energy/constant_model.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/memory.hpp>
#include <iostream>
#include <stdexcept>
#include <string>


namespace dcs { namespace eesim {

template <typename TraitsT>
class physical_resource
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	//public: typedef ::dcs::perfeval::energy::any_model<real_type> energy_model_type;
	public: typedef ::dcs::perfeval::energy::base_model<real_type> energy_model_type;
	public: typedef ::dcs::shared_ptr<energy_model_type> energy_model_pointer;


	public: physical_resource()
	: name_("Unnamed Resource"),
	  category_(),//FIXME: need a dummy value
	  capacity_(0),
	  max_util_(0),
//	  energy_(::dcs::perfeval::energy::make_any_model(::dcs::perfeval::energy::constant_model<real_type>(0)))
	  ptr_energy_(new ::dcs::perfeval::energy::constant_model<real_type>(0))
	{
	}


	public: physical_resource(::std::string const& name,
							  physical_resource_category category,
							  real_type capacity,
							  real_type max_util = real_type(1))
		: name_(name),
		  category_(category),
		  capacity_(capacity),
		  max_util_(max_util),
//		  energy_(::dcs::perfeval::energy::make_any_model(::dcs::perfeval::energy::constant_model<real_type>(0)))
		  ptr_energy_(new ::dcs::perfeval::energy::constant_model<real_type>(0))
	{
		// check preconditions
		assert_check_capacity(capacity_);
		assert_check_max_util(max_util_);
	}


	public: template <typename EnergyModelT>
		physical_resource(::std::string const& name,
						  physical_resource_category category,
						  real_type capacity,
						  real_type max_util,
//						  EnergyModelT const& energy_model)
						  energy_model_pointer const& ptr_energy_model)
		: name_(name),
		  category_(category),
		  capacity_(capacity),
		  max_util_(max_util),
//		  energy_(::dcs::perfeval::energy::make_any_model(energy_model))
		  ptr_energy_(ptr_energy_model)
	{
		// check preconditions
		assert_check_capacity(capacity_);
		assert_check_max_util(max_util_);
	}


	public: void name(::std::string const& name)
	{
		name_ = name;
	}


	public: ::std::string const& name() const
	{
		return name_;
	}


	public: void category(physical_resource_category category)
	{
		category_ = category;
	}


	public: physical_resource_category category() const
	{
		return category_;
	}


	public: void capacity(real_type c)
	{
		// check precondition
		assert_check_capacity(c);

		capacity_ = c;
	}


	public: real_type capacity() const
	{
		return capacity_;
	}


	public: void utilization_threshold(real_type p)
	{
		// check precondition
		assert_check_max_util(p);

		max_util_ = p;
	}


	public: real_type utilization_threshold() const
	{
		return max_util_;
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


//	public: energy_model_type& energy_model()
//	{
//		return energy_;
//	}


//	public: energy_model_type const& energy_model() const
//	{
//		return energy_;
//	}


	private: void assert_check_capacity(real_type c)
	{
		// precondition: c must be >= 0.
		DCS_ASSERT(
			c >= 0,
			throw ::std::domain_error("[dcs::eesim::physical_machine::assert_check_capacity] Input value is out-of-range.")
		);
	}


	private: void assert_check_max_util(real_type p)
	{
		// precondition: p must be in the range [0,1].
		DCS_ASSERT(
			p >= 0 && p <= 1,
			throw ::std::domain_error("[dcs::eesim::physical_machine::assert_check_max_util] Input value is out-of-range.")
		);
	}


	private: ::std::string name_;
	private: physical_resource_category category_;
	private: real_type capacity_;
	/// Upper-bound on maximum utilization (in percentage)
	private: real_type max_util_;
	//private: energy_model_type energy_;
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
