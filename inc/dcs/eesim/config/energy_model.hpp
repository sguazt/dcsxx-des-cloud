/**
 * \file dcs/eesim/config/energy_model.hpp
 *
 * \brief Configuration for energy models.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright (C) 2009-2012  Marco Guazzone (marco.guazzone@gmail.com)
 *                          [Distributed Computing System (DCS) Group,
 *                           Computer Science Institute,
 *                           Department of Science and Technological Innovation,
 *                           University of Piemonte Orientale,
 *                           Alessandria (Italy)]
 *
 * This file is part of dcsxx-des-cloud.
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
 */

#ifndef DCS_EESIM_CONFIG_ENERGY_MODEL_HPP
#define DCS_EESIM_CONFIG_ENERGY_MODEL_HPP


#include <iosfwd>


namespace dcs { namespace eesim { namespace config {

enum energy_model_category
{
	constant_energy_model,
	fan2007_energy_model
};


template <typename RealT>
struct constant_energy_model_config
{
	typedef RealT real_type;

	real_type c0;
};


template <typename RealT>
struct fan2007_energy_model_config
{
	typedef RealT real_type;

	real_type c0;
	real_type c1;
	real_type c2;
	real_type r;
};


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, constant_energy_model_config<RealT> const& model)
{
	return os << "<(constant_energy_model)"
			  << " c0: " << model.c0
			  << ">";
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, fan2007_energy_model_config<RealT> const& model)
{
	return os << "<(fan2007_energy_model)"
			  << " c0: " << model.c0
			  << ", c1: " << model.c1
			  << ", c2: " << model.c2
			  << ", r: " << model.r
			  << ">";
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_ENERGY_MODEL_HPP
