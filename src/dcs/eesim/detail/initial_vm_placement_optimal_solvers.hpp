#ifndef DCS_EESIM_DETAIL_INITIAL_VM_PLACEMENT_OPTIMAL_SOLVERS_HPP
#define DCS_EESIM_DETAIL_INITIAL_VM_PLACEMENT_OPTIMAL_SOLVERS_HPP


#include <dcs/eesim/detail/ampl/vm_placement_minlp_solver.hpp>
#include <dcs/eesim/detail/base_initial_vm_placement_optimal_solver.hpp>
#include <dcs/eesim/detail/neos/vm_placement_minlp_solver.hpp>
#include <dcs/eesim/optimal_solver_categories.hpp>
#include <dcs/eesim/optimal_solver_input_methods.hpp>
#include <dcs/eesim/optimal_solver_params.hpp>
#include <dcs/eesim/optimal_solver_proxies.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim { namespace detail {

template <typename TraitsT>
::dcs::shared_ptr< base_initial_vm_placement_optimal_solver<TraitsT> > make_initial_vm_placement_optimal_solver(base_optimal_solver_params<TraitsT> const& params)
{
	::dcs::shared_ptr< base_initial_vm_placement_optimal_solver<TraitsT> > ptr_solver;

	switch (params.category())
	{
		case minco_optimal_solver_category:
			switch (params.proxy())
			{
				case neos_optimal_solver_proxy:
					ptr_solver = ::dcs::make_shared< neos::initial_vm_placement_minlp_solver<TraitsT> >(
										params.solver_id(),
										params.input_method()
									);
					break;
				case none_optimal_solver_proxy:
					switch (params.input_method())
					{
						case ampl_optimal_solver_input_method:
							ptr_solver = ::dcs::make_shared< ampl::initial_vm_placement_minlp_solver<TraitsT> >(
												params.solver_id()
											);
							break;
//TODO
//						case gams_optimal_solver_input_method:
//							ptr_solver = ::dcs::make_shared< gams::initial_vm_placement_minlp_solver<TraitsT> >(
//												params.solver_id()
//											);
//							break;
						default:
							throw ::std::runtime_error("[dcs::eesim::detail::make_initial_vm_placement_optimal_solver] Solver input method not handled.");
					}
					break;
				default:
					throw ::std::runtime_error("[dcs::eesim::detail::make_initial_vm_placement_optimal_solver] Solver proxy not handled.");
			}
			break;
		default:
			throw ::std::runtime_error("[dcs::eesim::detail::make_initial_vm_placement_optimal_solver] Solver category not handled.");
	}

	return ptr_solver;
}

}}} // Namespace dcs::eesim::detail


#endif // DCS_EESIM_DETAIL_INITIAL_VM_PLACEMENT_OPTIMAL_SOLVERS_HPP
