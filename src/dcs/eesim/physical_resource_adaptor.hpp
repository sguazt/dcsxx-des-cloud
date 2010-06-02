/**
 * \file dcs/eesim/physical_resource_adaptor.hpp
 *
 * \brief Adaptor class for physical resources.
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

#ifndef DCS_EESIM_PHYSICAL_MACHINE_ADAPTOR_HPP
#define DCS_EESIM_PHYSICAL_MACHINE_ADAPTOR_HPP


#include <dcs/perfeval/energy/any_model.hpp>
#include <dcs/eesim/base_physical_resource.hpp>
#include <dcs/eesim/physical_resource.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/type_traits/add_reference.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/remove_reference.hpp>
#include <vector>


namespace dcs { namespace eesim {

/**
 * \brief Adaptor class for physical resources.
 *
 * \tparam PhysicalMachineT The adaptee physical resource class type.
 * \tparam PhysicalMachineTraitsT Type traits for the adaptee physical resource
 *  class type.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <
	typename PhysicalResourceT,
	typename PhysicalResourceTraitsT=typename ::dcs::type_traits::remove_reference<PhysicalResourceT>::type
>
class physical_resource_adaptor: public base_physical_resource<
										typename PhysicalResourceTraitsT::traits_type
								>
{
	public: typedef PhysicalResourceT physical_resource_type;
	public: typedef typename PhysicalResourceTraitsT::traits_type traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef ::dcs::perfeval::energy::any_model<real_type> energy_model_type;
	public: typedef physical_resource<traits_type> physical_resource_type;
	private: typedef typename ::dcs::type_traits::add_reference<physical_resource_type>::type resource_reference;
	private: typedef typename ::dcs::type_traits::add_reference<typename ::dcs::type_traits::add_const<physical_resource_type>::type>::type resource_const_reference;


	public: physical_resource_adaptor(resource_const_reference adaptee_resource)
		: resource_(adaptee_resource)
	{
	}


	public: ::std::string const& name() const
	{
		return resource_.name();
	}


	public: physical_resource_category category() const
	{
		return resource_.category();
	}


	public: real_type consumed_energy(real_type u) const
	{
		return resource_.consumed_energy(u);
	}


	private: physical_resource_type resource_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_PHYSICAL_MACHINE_ADAPTOR_HPP
