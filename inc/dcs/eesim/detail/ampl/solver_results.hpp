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
