/**
 * \file dcs/des/cloud/physical_resource_adaptor.hpp
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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_CLOUD_PHYSICAL_RESOURCE_ADAPTOR_HPP
#define DCS_DES_CLOUD_PHYSICAL_RESOURCE_ADAPTOR_HPP


#include <dcs/perfeval/energy/any_model.hpp>
#include <dcs/des/cloud/base_physical_resource.hpp>
#include <dcs/des/cloud/physical_resource.hpp>
#include <dcs/des/cloud/physical_resource_category.hpp>
#include <dcs/exception.hpp>
#include <dcs/macro.hpp>
#include <dcs/type_traits/add_reference.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/remove_reference.hpp>
#include <stdexcept>
#include <vector>


namespace dcs { namespace des { namespace cloud {

/**
 * \brief Adaptor class for physical resources.
 *
 * \tparam PhysicalMachineT The adaptee physical resource class type.
 * \tparam PhysicalMachineTraitsT Type traits for the adaptee physical resource
 *  class type.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
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


	/// Copy constructor.
	private: physical_resource_adaptor(physical_resource_adaptor const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: physical_resource_adaptor& operator=(physical_resource_adaptor const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
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

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_PHYSICAL_RESOURCE_ADAPTOR_HPP
