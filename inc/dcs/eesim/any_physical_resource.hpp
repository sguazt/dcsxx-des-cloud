/**
 * \file dcs/eesim/any_physical_resource.hpp
 *
 * \brief Generic (type-erased) class for physical resources.
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

#ifndef DCS_EESIM_ANY_PHYSICAL_RESOURCE_HPP
#define DCS_EESIM_ANY_PHYSICAL_RESOURCE_HPP


#include <dcs/eesim/base_physical_resource.hpp>
#include <dcs/eesim/physical_resource_adaptor.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/exception.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <dcs/util/holder.hpp>
#include <iostream>
#include <stdexcept>


namespace dcs { namespace eesim {

template <typename TraitsT>
class any_physical_resource
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;


	/// Default constructor.
	public: any_physical_resource()
	{
	}


	public: template <typename PhysicalResourceT>
		explicit any_physical_resource(PhysicalResourceT const& machine)
		: ptr_resource_(new physical_resource_adaptor<PhysicalResourceT>(machine))
	{
	}


	public: template <typename PhysicalResourceT>
		explicit any_physical_resource(::dcs::util::holder<PhysicalResourceT> const& wrap_resource)
		: ptr_resource_(new physical_resource_adaptor<PhysicalResourceT>(wrap_resource.get()))
	{
	}


	/// Copy constructor.
	private: any_physical_resource(any_physical_resource const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: any_physical_resource& operator=(any_physical_resource const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
	}


	public: template <typename PhysicalResourceT>
		void physical_resource(PhysicalResourceT resource)
	{
		ptr_resource_ = new physical_resource_adaptor<PhysicalResourceT>(resource);
	}


	public: ::std::string const& name() const
	{
		return ptr_machine_->name();
	}


	public: real_type consumed_energy(real_type u) const
	{
		return ptr_machine_->consumed_energy();
	}


	private: ::dcs::shared_ptr<base_physical_resource_type> ptr_resource_;
};


template <
	typename PhysicalResourceT,
	typename PhysicalResourceTraitsT=typename ::dcs::type_traits::remove_reference<PhysicalResourceT>::type
>
struct make_any_physical_resource_type
{
	typedef any_physical_resource<typename PhysicalResourceTraitsT::traits_type> type;
};


namespace detail {

template <
	typename PhysicalResourceT,
	typename PhysicalResourceTraitsT=typename ::dcs::type_traits::remove_reference<PhysicalResourceT>::type
>
struct make_any_physical_resource_impl;


template <typename PhysicalResourceT, typename PhysicalResourceTraitsT>
struct make_any_physical_resource_impl
{
	typedef typename make_any_physical_machine_type<PhysicalResourceT,PhysicalResourceTraitsT>::type any_physical_machine_type;
	static any_physical_resource_type apply(PhysicalResourceT& machine)
	{
		return any_physical_resource_type(machine);
	}
};


template <typename PhysicalResourceT, typename PhysicalResourceTraitsT>
struct make_any_physical_resource_impl<PhysicalResourceT&,PhysicalResourceTraitsT>
{
	typedef typename make_any_physical_machine_type<PhysicalResourceT,PhysicalResourceTraitsT>::type any_physical_resource_type;
	static any_physical_resource_type apply(PhysicalResourceT& resource)
	{
		::dcs::util::holder<PhysicalResourceT&> wrap_resource(resource);
		return any_physical_resource_type(wrap_resource);
	}
};

} // Namespace detail


template <typename PhysicalResourceT>
typename make_any_physical_resource_type<PhysicalResourceT>::type make_any_physical_resource(PhysicalResourceT resource)
{
	return detail::make_any_physical_resource_impl<PhysicalResourceT>::apply(resource);
}



template <
	typename CharT,
	typename CharTraitsT,
	typename TraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, any_physical_resource<TraitsT> const& resource)
{
	//typedef any_physical_resource<TraitsT> resource_type;

	os << "<Name: " << resource.name()
	   << ">";

	return os;
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_ANY_PHYSICAL_RESOURCE_HPP
