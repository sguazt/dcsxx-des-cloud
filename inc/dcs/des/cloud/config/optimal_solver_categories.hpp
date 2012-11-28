/**
 * \file dcs/des/cloud/config/optimal_solver_categories.hpp
 *
 * \brief Configuration for optimal solver categories.
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

#ifndef DCS_DES_CLOUD_CONFIG_OPTIMAL_SOLVER_CATEGORIES_HPP
#define DCS_DES_CLOUD_CONFIG_OPTIMAL_SOLVER_CATEGORIES_HPP


#include <dcs/des/cloud/optimal_solver_categories.hpp>
#include <iosfwd>


namespace dcs { namespace des { namespace cloud { namespace config {

typedef ::dcs::des::cloud::optimal_solver_categories optimal_solver_categories;

/*
enum optimal_solver_categories
{
	bco_optimal_solver_category, ///< Bound Constrained Optimization
	co_optimal_solver_category, ///< Combinatorial Optimization and Integer Programming
	cp_optimal_solver_category, ///< Complementarity Problems
	go_optimal_solver_category, ///< Global Optimization
	kestrel_optimal_solver_category, ///< Kestrel
	lno_optimal_solver_category, ///< Linear Network Programming
	lp_optimal_solver_category, ///< Linear Programming
	milp_optimal_solver_category, ///< Mixed Integer Linear Programming
	minco_optimal_solver_category, ///< Mixed Integer Nonlinearly Constrained Optimization
	multi_optimal_solver_category, ///< Multi-Solvers
	nco_optimal_solver_category, ///< Nonlinearly Constrained Optimization
	ndo_optimal_solver_category, ///< Nondifferentiable Optimization
	sdp_optimal_solver_category, ///< Semidefinite Programming
	sio_optimal_solver_category, ///< Semi-infinite Optimization
	slp_optimal_solver_category, ///< Stochastic Linear Programming
	socp_optimal_solver_category, ///< Second Order Conic Programming
	uco_optimal_solver_category ///< Unconstrained Optimization
};
*/

template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, optimal_solver_categories category)
{
	switch (category)
	{
		case bco_optimal_solver_category: ///< Bound Constrained Optimization
			os << "bco";
			break;
		case co_optimal_solver_category: ///< Combinatorial Optimization and Integer Programming
			os << "co";
			break;
		case cp_optimal_solver_category: ///< Complementarity Problems
			os << "cp";
			break;
		case go_optimal_solver_category: ///< Global Optimization
			os << "go";
			break;
		case kestrel_optimal_solver_category: ///< Kestrel
			os << "kestrel";
			break;
		case lno_optimal_solver_category: ///< Linear Network Programming
			os << "lno";
			break;
		case lp_optimal_solver_category: ///< Linear Programming
			os << "lp";
			break;
		case milp_optimal_solver_category: ///< Mixed Integer Linear Programming
			os << "milp";
			break;
		case minco_optimal_solver_category: ///< Mixed Integer Nonlinearly Constrained Optimization
			os << "minco";
			break;
		case multi_optimal_solver_category: ///< Multi-Solvers
			os << "multi";
			break;
		case nco_optimal_solver_category: ///< Nonlinearly Constrained Optimization
			os << "nco";
			break;
		case ndo_optimal_solver_category: ///< Nondifferentiable Optimization
			os << "ndo";
			break;
		case sdp_optimal_solver_category: ///< Semidefinite Programming
			os << "sdp";
			break;
		case sio_optimal_solver_category: ///< Semi-infinite Optimization
			os << "sio";
			break;
		case slp_optimal_solver_category: ///< Stochastic Linear Programming
			os << "slp";
			break;
		case socp_optimal_solver_category: ///< Second Order Conic Programming
			os << "socp";
			break;
		case uco_optimal_solver_category: ///< Unconstrained Optimization
			os << "uco";
			break;
	}

	return os;
}


}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPTIMAL_SOLVER_CATEGORIES_HPP
