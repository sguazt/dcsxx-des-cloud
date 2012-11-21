#ifndef DCS_EESIM_CONFIG_OPTIMAL_SOLVER_CATEGORIES_HPP
#define DCS_EESIM_CONFIG_OPTIMAL_SOLVER_CATEGORIES_HPP


#include <dcs/eesim/optimal_solver_categories.hpp>
#include <iosfwd>


namespace dcs { namespace eesim { namespace config {

typedef ::dcs::eesim::optimal_solver_categories optimal_solver_categories;

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


}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPTIMAL_SOLVER_CATEGORIES_HPP
