#ifndef DCS_EESIM_DETAIL_VM_PLACEMENT_OPTIMAL_SOLVERS_HPP
#define DCS_EESIM_DETAIL_VM_PLACEMENT_OPTIMAL_SOLVERS_HPP


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
