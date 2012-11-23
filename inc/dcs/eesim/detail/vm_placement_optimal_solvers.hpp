/**
 * \file dcs/eesim/detail/vm_placement_optmal_solvers.hpp
 *
 * \brief Utilities for VM placement optimal solvers.
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

#ifndef DCS_EESIM_DETAIL_VM_PLACEMENT_OPTIMAL_SOLVERS_HPP
#define DCS_EESIM_DETAIL_VM_PLACEMENT_OPTIMAL_SOLVERS_HPP


#include <dcs/eesim/detail/ampl/vm_placement_minlp_solver.hpp>
#include <dcs/eesim/detail/base_vm_placement_optimal_solver.hpp>
#include <dcs/eesim/detail/neos/vm_placement_minlp_solver.hpp>
#include <dcs/eesim/optimal_solver_categories.hpp>
#include <dcs/eesim/optimal_solver_params.hpp>
#include <dcs/eesim/optimal_solver_proxies.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim { namespace detail {

template <typename TraitsT>
::dcs::shared_ptr< base_vm_placement_optimal_solver<TraitsT> > make_vm_placement_optimal_solver(base_optimal_solver_params<TraitsT> const& params)
{
	::dcs::shared_ptr< base_vm_placement_optimal_solver<TraitsT> > ptr_solver;

	switch (params.category())
	{
		case minco_optimal_solver_category:
			switch (params.proxy())
			{
				case neos_optimal_solver_proxy:
					ptr_solver = ::dcs::make_shared< neos::vm_placement_minlp_solver<TraitsT> >(
										params.solver_id(),
										params.input_method()
									);
					break;
				case none_optimal_solver_proxy:
					switch (params.input_method())
					{
						case ampl_optimal_solver_input_method:
							ptr_solver = ::dcs::make_shared< ampl::vm_placement_minlp_solver<TraitsT> >(
												params.solver_id()
											);
							break;
//TODO
//						case gams_optimal_solver_input_method:
//							ptr_solver = ::dcs::make_shared< gams::vm_placement_minlp_solver<TraitsT> >(
//												params.solver_id()
//											);
//							break;
						default:
							throw ::std::runtime_error("[dcs::eesim::detail::make_vm_placement_optimal_solver] Solver input method not handled.");
					}
					break;
				default:
					throw ::std::runtime_error("[dcs::eesim::detail::make_vm_placement_optimal_solver] Solver proxy not handled.");
			}
			break;
		default:
			throw ::std::runtime_error("[dcs::eesim::detail::make_vm_placement_optimal_solver] Solver category not handled.");
	}

	return ptr_solver;
}

}}} // Namespace dcs::eesim::detail


#endif // DCS_EESIM_DETAIL_VM_PLACEMENT_OPTIMAL_SOLVERS_HPP
