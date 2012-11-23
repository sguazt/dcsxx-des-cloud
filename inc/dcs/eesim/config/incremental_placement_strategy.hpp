/**
 * \file dcs/eesim/config/incremental_placement_strategy.hpp
 *
 * \brief Configuration for incremental placement strategies.
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

#ifndef DCS_EESIM_CONFIG_INCREMENTAL_PLACEMENT_STRATEGY_HPP
#define DCS_EESIM_CONFIG_INCREMENTAL_PLACEMENT_STRATEGY_HPP


#include <boost/variant.hpp>
#include <dcs/macro.hpp>
#include <iosfwd>


namespace dcs { namespace eesim { namespace config {

enum incremental_placement_strategy_category
{
	best_fit_incremental_placement_strategy,
	best_fit_decreasing_incremental_placement_strategy
};


struct best_fit_incremental_placement_strategy_config
{
};


struct best_fit_decreasing_incremental_placement_strategy_config
{
};


template <typename RealT>
struct incremental_placement_strategy_config
{
	typedef RealT real_type;
    typedef best_fit_incremental_placement_strategy_config best_fit_incremental_placement_strategy_config_type;
    typedef best_fit_decreasing_incremental_placement_strategy_config best_fit_decreasing_incremental_placement_strategy_config_type;


	incremental_placement_strategy_category category;
    ::boost::variant<best_fit_incremental_placement_strategy_config_type,
    				 best_fit_decreasing_incremental_placement_strategy_config_type> category_conf;
	real_type ref_penalty;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, incremental_placement_strategy_category const& category)
{
	switch (category)
	{
		case best_fit_incremental_placement_strategy:
			os << "best-fit";
			break;
		case best_fit_decreasing_incremental_placement_strategy:
			os << "best-fit-decreasing";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, best_fit_incremental_placement_strategy_config const& strategy)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( strategy );

	os << "<(best-fit-incremental-placement)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, best_fit_decreasing_incremental_placement_strategy_config const& strategy)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( strategy );

	os << "<(best-fit-decreasing-incremental-placement)>";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, incremental_placement_strategy_config<RealT> const& strategy)
{
	os << strategy.category_conf;

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_INCREMENTAL_PLACEMENT_STRATEGY_HPP
