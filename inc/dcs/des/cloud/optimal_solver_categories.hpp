/**
 * \file dcs/des/cloud/optimal_solver_categories.hpp
 *
 * \brief Categories of optimal mathematical solvers.
 *
 * Copyright (C) 2009-2011  Distributed Computing System (DCS) Group, Computer
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

#ifndef DCS_DES_CLOUD_OPTIMAL_SOLVER_CATEGORIES_HPP
#define DCS_DES_CLOUD_OPTIMAL_SOLVER_CATEGORIES_HPP

namespace dcs { namespace des { namespace cloud {

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

}}} // Namespace dcs::des::cloud

#endif // DCS_DES_CLOUD_OPTIMAL_SOLVER_CATEGORIES_HPP
