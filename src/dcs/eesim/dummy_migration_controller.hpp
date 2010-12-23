/**
 * \file src/dcs/eesim/dummy_migration_controller.hpp
 *
 * \brief Dummy Migration Controller.
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

#ifndef DCS_EESIM_DUMMY_MIGRATION_CONTROLLER_HPP
#define DCS_EESIM_DUMMY_MIGRATION_CONTROLLER_HPP


#include <dcs/debug.hpp>
#include <dcs/eesim/base_migration_controller.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT>
class dummy_migration_controller: public base_migration_controller<TraitsT>
{
	private: typedef base_migration_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;


	public: dummy_migration_controller()
	: base_type()
	{
	}
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_DUMMY_MIGRATION_CONTROLLER_HPP
