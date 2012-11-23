/**
 * \file dcs/eesim/detail/ampl/solver_result.hpp
 *
 * \brief Categories for the results of optimal VM placement strategies
 *  based on the AMPL mathematical environment.
 *
 * Copyright (C) 2009-2012  Distributed Computing System (DCS) Group, Computer
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

#ifndef DCS_EESIM_DETAIL_AMPL_SOLVER_RESULTS_HPP
#define DCS_EESIM_DETAIL_AMPL_SOLVER_RESULTS_HPP


#include <string>


namespace dcs { namespace eesim { namespace detail { namespace ampl {

enum solver_results
{
	solved_result, ///< Optimal solution found.
	maybe_solved_result, ///< Optimal solution indicated, but error likely.
	infeasible_result, ///< Constraints cannot be satisfied.
	unbounded_result, ///< Objective can be improved without limit.
	limit_result, ///< Stopped by a limit that the user set (such as on iterations).
	failure_result, ///< Stopped by an error condition in the solver.
	unknown_result ///< Fall-back case.
};

inline
solver_results solver_result_from_string(::std::string const& str)
{
	if (!str.compare("solved"))
	{
		return solved_result;
	}
	if (!str.compare("solved?"))
	{
		return maybe_solved_result;
	}
	if (!str.compare("infeasible"))
	{
		return infeasible_result;
	}
	if (!str.compare("unbounded"))
	{
		return unbounded_result;
	}
	if (!str.compare("limit"))
	{
		return limit_result;
	}
	if (!str.compare("failure"))
	{
		return failure_result;
	}

	return unknown_result;
}

}}}} // Namespace dcs::eesim::detail::ampl


#endif // DCS_EESIM_DETAIL_AMPL_SOLVER_RESULTS_HPP
