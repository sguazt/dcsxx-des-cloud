/**
 * \file dcs/des/cloud/traits.hpp
 *
 * \brief Type traits class.
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

#ifndef DCS_DES_CLOUD_TRAITS_HPP
#define DCS_DES_CLOUD_TRAITS_HPP


#include <cstddef>


namespace dcs { namespace des { namespace cloud {

template <
	typename DesEngineT,
	typename UniformRandomGeneratorT,
	typename ConfigurationT,
	typename RealT,
	typename UIntT,
	typename IntT
>
struct traits
{
	typedef DesEngineT des_engine_type;
	typedef UniformRandomGeneratorT uniform_random_generator_type;
	typedef ConfigurationT configuration_type;
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef IntT int_type;
	typedef long application_identifier_type;
	typedef long physical_machine_identifier_type;
	typedef long virtual_machine_identifier_type;

	static const application_identifier_type invalid_application_id = -1;
	static const physical_machine_identifier_type invalid_physical_machine_id = -1;
	static const virtual_machine_identifier_type invalid_virtual_machine_id = -1;
};

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_TRAITS_HPP
