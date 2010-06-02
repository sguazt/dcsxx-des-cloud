/**
 * \file dcs/eesim/traits.hpp
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
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_EESIM_TRAITS_HPP
#define DCS_EESIM_TRAITS_HPP


namespace dcs { namespace eesim {

template <
	typename DesEngineT,
	typename UniformRandomGeneratorT,
	typename RealT,
	typename UIntT
>
struct traits
{
	typedef DesEngineT des_engine_type;
	typedef UniformRandomGeneratorT uniform_random_generator_type;
	typedef RealT real_type;
	typedef UIntT uint_type;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_TRAITS_HPP
