#ifndef DCS_EESIM_OPTIMAL_SOLVER_CATEGORIES_HPP
#define DCS_EESIM_OPTIMAL_SOLVER_CATEGORIES_HPP

namespace dcs { namespace eesim {

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

}} // Namespace dcs::eesim

#endif // DCS_EESIM_OPTIMAL_SOLVER_CATEGORIES_HPP
