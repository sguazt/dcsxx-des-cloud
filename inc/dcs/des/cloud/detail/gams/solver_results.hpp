/**
 * \file dcs/eesim/detail/gams/vm_placement_problem_result.hpp
 *
 * \brief Result categories for optimal solvers based on the GAMS mathematical
 *  environment.
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

#ifndef DCS_EESIM_DETAIL_GAMS_SOLVER_RESULTS_HPP
#define DCS_EESIM_DETAIL_GAMS_SOLVER_RESULTS_HPP


namespace dcs { namespace eesim { namespace detail { namespace gams {

enum solver_results
{
	normal_completion_solver_result = 1, ///< The solver terminated in a normal way.
	iteration_interrupt_solver_result, ///< The solver was interrupted because it used too many iterations.
	resource_interrupt_solver_result, ///< The solver was interrupted because it used too much time.
	terminated_by_solver_solver_result, ///< The solver encountered difficulty and was unable to continue.
	evaluation_error_limit_solver_result, ///< Too many evaluations of nonlinear terms at undefined values.
	capability_problems_solver_result, ///< The solver does not have the capability required by the model.
	licensing_problem_solver_result, ///< The solver cannot find the appropriate license key needed to use a specific subsolver.
	user_interrupt_solver_result, ///< The user has sent a message to interrupt the solver.
	error_setup_failure_solver_result, ///< The solver encountered a fatal failure during problem set-up time.
	error_solver_failure_solver_result, ///< The solver encountered a fatal error.
	error_internal_solver_failure_solver_result, ///< The solver encountered an internal fatal error.
	solver_processing_skipped_solver_result, ///< The entire solve step has been skipped.
	error_system_failure_solver_result, ///< This indicates a completely unknown or unexpected error condition.
	unknown_solver_result ///< Fall-back case.
};

}}}} // Namespace dcs::eesim::detail::gams


#endif // DCS_EESIM_DETAIL_GAMS_SOLVER_RESULTS_HPP
