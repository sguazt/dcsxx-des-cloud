/**
 * \file dcs/des/cloud/config/initial_placement_strategy.hpp
 *
 * \brief Configuration for initial placement strategies.
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

#ifndef DCS_DES_CLOUD_CONFIG_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_DES_CLOUD_CONFIG_INITIAL_PLACEMENT_STRATEGY_HPP


#include <boost/variant.hpp>
#include <dcs/des/cloud/optimal_solver_categories.hpp>
#include <dcs/des/cloud/optimal_solver_ids.hpp>
#include <dcs/des/cloud/optimal_solver_input_methods.hpp>
#include <dcs/des/cloud/optimal_solver_proxies.hpp>
#include <dcs/macro.hpp>
#include <iosfwd>


namespace dcs { namespace des { namespace cloud { namespace config {

enum initial_placement_strategy_category
{
	best_fit_initial_placement_strategy,
	best_fit_decreasing_initial_placement_strategy,
	first_fit_initial_placement_strategy,
	first_fit_scaleout_initial_placement_strategy,
	optimal_initial_placement_strategy
};


struct best_fit_initial_placement_strategy_config
{
};


struct best_fit_decreasing_initial_placement_strategy_config
{
};


struct first_fit_initial_placement_strategy_config
{
};


struct first_fit_scaleout_initial_placement_strategy_config
{
};


template <typename RealT>
struct optimal_initial_placement_strategy_config
{
	typedef RealT real_type;

	real_type wp;
	real_type ws;
	optimal_solver_categories category;
	optimal_solver_input_methods input_method;
	optimal_solver_ids solver_id;
	optimal_solver_proxies proxy;
};


template <typename RealT>
struct initial_placement_strategy_config
{
	typedef RealT real_type;
    typedef best_fit_initial_placement_strategy_config best_fit_initial_placement_strategy_config_type;
    typedef best_fit_decreasing_initial_placement_strategy_config best_fit_decreasing_initial_placement_strategy_config_type;
    typedef first_fit_initial_placement_strategy_config first_fit_initial_placement_strategy_config_type;
    typedef first_fit_scaleout_initial_placement_strategy_config first_fit_scaleout_initial_placement_strategy_config_type;
    typedef optimal_initial_placement_strategy_config<real_type> optimal_initial_placement_strategy_config_type;


	initial_placement_strategy_category category;
    ::boost::variant<best_fit_initial_placement_strategy_config_type,
					 best_fit_decreasing_initial_placement_strategy_config_type,
					 first_fit_initial_placement_strategy_config_type,
					 first_fit_scaleout_initial_placement_strategy_config_type,
					 optimal_initial_placement_strategy_config_type> category_conf;
	real_type ref_penalty;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, initial_placement_strategy_category const& category)
{
	switch (category)
	{
		case best_fit_initial_placement_strategy:
			os << "best-fit";
			break;
		case best_fit_decreasing_initial_placement_strategy:
			os << "best-fit-decreasing";
			break;
		case first_fit_initial_placement_strategy:
			os << "first-fit";
			break;
		case first_fit_scaleout_initial_placement_strategy:
			os << "first-fit-scaleout";
			break;
		case optimal_initial_placement_strategy:
			os << "optimal";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, best_fit_initial_placement_strategy_config const& strategy)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( strategy );

	os << "<(best-fit-initial-placement)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, best_fit_decreasing_initial_placement_strategy_config const& strategy)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( strategy );

	os << "<(best-fit-decreasing-initial-placement)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, first_fit_initial_placement_strategy_config const& strategy)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( strategy );

	os << "<(first-fit-initial-placement)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, first_fit_scaleout_initial_placement_strategy_config const& strategy)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( strategy );

	os << "<(first-fit-scaleout-initial-placement)>";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, optimal_initial_placement_strategy_config<RealT> const& strategy)
{
	os << "<(optimal-initial-placement)"
	   << " power-weight: " << strategy.wp
	   << ", sla-weight: " << strategy.ws
	   << ", category: " << strategy.category
	   << ", input: " << strategy.input_method
	   << ", solver: " << strategy.solver_id
	   << ", proxy: " << strategy.proxy
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, initial_placement_strategy_config<RealT> const& strategy)
{
	os << strategy.category_conf;

	return os;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_INITIAL_PLACEMENT_STRATEGY_HPP
