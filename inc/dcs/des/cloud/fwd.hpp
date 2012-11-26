/**
 * \file dcs/eesim/fwd.hpp
 *
 * \brief Forward declarations.
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

#ifndef DCS_EESIM_FWD_HPP
#define DCS_EESIM_FWD_HPP


namespace dcs { namespace eesim {

//template <typename TraitsT>
//class application_controller;


template <typename TraitsT>
class application_tier;


template <typename TraitsT>
class data_center;


template <typename TraitsT>
class data_center_manager;


//template <typename TraitsT>
//class migration_controller;


template <typename TraitsT>
class multi_tier_application;


//template <typename TraitsT>
//class physical_machine_controller;


template <typename TraitsT>
class physical_machine;


template <typename TraitsT>
class physical_resource;


template <typename TraitsT>
class registry;


template <typename TraitsT>
class virtual_machine;


template <typename TraitsT>
class virtual_machine_monitor;

}} // Namespace dcs::eesim


#endif // DCS_EESIM_FWD_HPP
