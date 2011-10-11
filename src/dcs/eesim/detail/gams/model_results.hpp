#ifndef DCS_EESIM_DETAIL_GAMS_MODEL_RESULTS_HPP
#define DCS_EESIM_DETAIL_GAMS_MODEL_RESULTS_HPP


namespace dcs { namespace eesim { namespace detail { namespace gams {

enum model_results
{
	optimal_model_result = 1, ///< This means that the solution is optimal. It only applies to linear problems or relaxed mixed integer problems (RMIP).
	locally_optimal_model_result, ///< This message means that a local optimum has been found. This is the message to look for if the problem is nonlinear, since all we can guarantee for general nonlinear problems is a local optimum.
	unbounded_optimal_model_result, ///< The solution is unbounded (This message is reliable if the problem is linear, but occasionally it appears for difficult nonlinear problems that are not truly unbounded, but that lack some strategically placed bounds to limit the variables to sensible values)
	infeasible_model_result, ///< This means that the linear problem is infeasible.
	locally_infeasible_model_result, ///< This message means that no feasible point could be found for the nonlinear problem from the given starting point. It does not necessarily mean that no feasible point exists.
	intermediate_infeasible_model_result, ///< This means that the current solution is not feasible, but that the solver program stopped, either because of a limit (iteration or resource), or because of some sort of difficulty.
	intermediate_nonoptimal_model_result, ///< This is again an incomplete solution, but it appears to be feasible.
	integer_solution_model_result, ///< An integer solution has been found to a MIP (mixed integer problem).
	intermediate_noninteger_model_result, ///< This is an incomplete solution to a MIP. An integer solution has not yet been found.
	integer_infeasible_model_result, ///< There is no integer solution to a MIP. This message should be reliable.
	lic_problem_no_solution_model_result, ///< The solver cannot find the appropriate license key needed to use a specific subsolver.
	error_unknown_model_result, ///< After a solver error, the model status is unknown.
	error_no_solution_model_result, ///< An error occurred and no solution has been returned. No solution will be returned to GAMS because of errors in the solution process.
	no_solution_required_model_result, ///< A solution is not expected for this solve. For example, the convert solver only reformats the model but does not give a solution.
	solved_unique_model_result, ///< This indicates a unique solution to a CNS model.
	solved_model_result, ///< A CNS model has been solved but multiple solutions may exist.
	solved_singular_model_result, ///< A CNS model has been solved but the point is singular.
	unbounded_no_solution_model_result, ///< The model is unbounded and no solution can be provided.
	infeasible_no_solution_model_result, ///< The model is infeasible and no solution can be provided.
	unknown_model_result ///< Fall-back case.
};

}}}} // Namespace dcs::eesim::detail::gams


#endif // DCS_EESIM_DETAIL_GAMS_MODEL_RESULTS_HPP
