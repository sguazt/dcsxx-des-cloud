/**
 * \file dcs/des/cloud/config/migration_controller.hpp
 *
 * \brief Configuration for migration controllers.
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

#ifndef DCS_DES_CLOUD_CONFIG_MIGRATION_CONTROLLER_HPP
#define DCS_DES_CLOUD_CONFIG_MIGRATION_CONTROLLER_HPP


#include <boost/variant.hpp>
#include <dcs/des/cloud/optimal_solver_categories.hpp>
#include <dcs/des/cloud/optimal_solver_ids.hpp>
#include <dcs/des/cloud/optimal_solver_input_methods.hpp>
#include <dcs/des/cloud/optimal_solver_proxies.hpp>
#include <dcs/macro.hpp>
#include <iosfwd>


namespace dcs { namespace des { namespace cloud { namespace config {

enum migration_controller_category
{
	best_fit_decreasing_migration_controller,
	dummy_migration_controller,
	optimal_migration_controller
};


struct best_fit_decreasing_migration_controller_config
{
};


struct dummy_migration_controller_config
{
};


template <typename RealT>
struct optimal_migration_controller_config
{
	typedef RealT real_type;

	real_type wp;
	real_type wm;
	real_type ws;
    optimal_solver_categories category;
    optimal_solver_input_methods input_method;
    optimal_solver_ids solver_id;
    optimal_solver_proxies proxy;
};


template <typename RealT>
struct migration_controller_config
{
	typedef RealT real_type;
	typedef best_fit_decreasing_migration_controller_config best_fit_decreasing_migration_controller_config_type;
	typedef dummy_migration_controller_config dummy_migration_controller_config_type;
	typedef optimal_migration_controller_config<real_type> optimal_migration_controller_config_type;

	real_type sampling_time;
	migration_controller_category category;
	::boost::variant<best_fit_decreasing_migration_controller_config_type,
					 dummy_migration_controller_config_type,
					 optimal_migration_controller_config_type> category_conf;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, migration_controller_category category)
{
	switch (category)
	{
		case best_fit_decreasing_migration_controller:
			os << "best-fit-decreasing";
			break;
		case dummy_migration_controller:
			os << "dummy";
			break;
		case optimal_migration_controller:
			os << "optimal";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, best_fit_decreasing_migration_controller_config const& conf)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( conf );

	os << "<(best-fit-decreasing-migration-controller)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, dummy_migration_controller_config const& conf)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( conf );

	os << "<(dummy-migration-controller)>";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, optimal_migration_controller_config<RealT> const& conf)
{
	os << "<(optimal-migration-controller)"
	   << " power-weight: " << conf.wp
	   << ", migration-weight: " << conf.wm
	   << ", sla-weight: " << conf.ws
	   << ", category: " << conf.category
	   << ", input: " << conf.input_method
	   << ", solver: " << conf.solver_id
	   << ", proxy: " << conf.proxy
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, migration_controller_config<RealT> const& controller)
{
	os << "<(migration-controller)"
	  << " sampling-time: " << controller.sampling_time
	  << ", " << controller.category_conf;

	return os;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_MIGRATION_CONTROLLER_HPP
