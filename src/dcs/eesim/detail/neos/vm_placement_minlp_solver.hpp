#ifndef DCS_EESIM_DETAIL_NEOS_VM_PLACEMENT_MINLP_SOLVER_HPP
#define DCS_EESIM_DETAIL_NEOS_VM_PLACEMENT_MINLP_SOLVER_HPP


#include <dcs/eesim/detail/ampl/solver_results.hpp>
#include <dcs/eesim/detail/ampl/vm_placement_problem.hpp>
#include <dcs/eesim/detail/ampl/vm_placement_problem_result.hpp>
#include <dcs/eesim/detail/gams/model_results.hpp>
#include <dcs/eesim/detail/gams/solver_results.hpp>
#include <dcs/eesim/detail/gams/vm_placement_problem.hpp>
#include <dcs/eesim/detail/gams/vm_placement_problem_result.hpp>
#include <dcs/eesim/detail/neos/client.hpp>
#include <dcs/eesim/detail/base_initial_vm_placement_optimal_solver.hpp>
#include <dcs/eesim/detail/base_vm_placement_optimal_solver.hpp>
#include <dcs/eesim/optimal_solver_categories.hpp>
#include <dcs/eesim/optimal_solver_ids.hpp>
#include <dcs/eesim/optimal_solver_input_methods.hpp>
#include <dcs/eesim/optimal_solver_proxies.hpp>
#include <string>


namespace dcs { namespace eesim { namespace detail { namespace neos {

namespace detail { namespace /*<unnamed>*/ {

inline
::std::string ampl_options()
{
	return	"option solution_precision 0;"
			"option solver_msg 0;"
			"option display_1col 0;"
			"option display_transpose 0;"
			"option gutter_width 1;"
			"option omit_zero_cols 0;"
			"option omit_zero_rows 0;"
			"option display_precision 0;"
			"option print_precision 0;"
			"solve;"
			"print '-- [RESULT] --';"
			"print 'solve_exitcode=', solve_exitcode, ';';"
			"print 'solve_result=', solve_result, ';';"
			"print 'solve_result_num=', solve_result_num, ';';"
			"print 'cost=', cost, ';';"
			"print 'x=[', {i in I} round(x[i]), '];';"
			"print 'y=[', {i in I} (({j in J} round(y[i,j])), ';'), '];';"
			"print 's=[', {i in I} (({j in J} s[i,j]), ';'), '];';"
			"print '-- [/RESULT] --';";
}

inline
::std::string gams_options()
{
	return	"";
}

}} // Namespace detail::<unnamed>


template <typename TraitsT>
class initial_vm_placement_minlp_solver: public base_initial_vm_placement_optimal_solver<TraitsT>
{
	private: typedef base_initial_vm_placement_optimal_solver<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	private: typedef typename base_type::real_type real_type;
	private: typedef typename base_type::data_center_type data_center_type;
	private: typedef typename base_type::virtual_machine_utilization_map virtual_machine_utilization_map;


	public: static const optimal_solver_input_methods default_input_method;
	public: static const optimal_solver_ids default_solver_id;


	public: explicit initial_vm_placement_minlp_solver(optimal_solver_ids solver_id = default_solver_id,
													   optimal_solver_input_methods method = default_input_method)
	: base_type(solver_id, method)
	{
		init();
	}


	private: optimal_solver_categories do_category() const
	{
		return minco_optimal_solver_category;
	}


	private: optimal_solver_proxies do_proxy() const
	{
		return neos_optimal_solver_proxy;
	}


    private: void do_solve(data_center_type const& dc,
						   real_type wp,
						   real_type ws,
						   real_type ref_penalty,
						   virtual_machine_utilization_map const& vm_util_map)
    {
		// Reset previous solution
		this->result().reset();

		switch (this->input_method())
		{
			case ampl_optimal_solver_input_method:
				{
					namespace ampl = ::dcs::eesim::detail::ampl;

					// Create a new problem
					// 1. Create a problem in AMPL format
					ampl::vm_placement_problem<traits_type> problem_descr;
					problem_descr = ampl::make_initial_vm_placement_problem<traits_type>(dc, wp, ws, ref_penalty, vm_util_map);
					::std::string xml_job;
					xml_job = make_ampl_job(xml_tmpl_,
										    problem_descr.model,
										    problem_descr.data,
										    detail::ampl_options());
					client neos;
					::std::string res;
//					res = execute_ampl_job(neos,
//										   minco_solver_category,
//										   couenne_solver_id,
//										   //minlp_solver_id,// license expired
//										   //filmint_solver_id,
//										   problem_descr.model,
//										   problem_descr.data,
//										   detail::ampl_options());
					res = execute_job(neos, xml_job);
					ampl::vm_placement_problem_result problem_res;
					problem_res = ampl::make_vm_placement_problem_result(res);
::std::cerr << "solver_exit_code: " << problem_res.solver_exit_code() << std::endl;//XXX
::std::cerr << "solver_result: " << problem_res.solver_result() << std::endl;//XXX
::std::cerr << "solver_result_code: " << problem_res.solver_result_code() << std::endl;//XXX
::std::cerr << "cost: " << problem_res.cost() << std::endl;//XXX
::std::cerr << "physical_machine_selection: " << problem_res.physical_machine_selection() << std::endl;//XXX
::std::cerr << "virtual_machine_placement: " << problem_res.virtual_machine_placement() << std::endl;//XXX
::std::cerr << "virtual_machine_shares: " << problem_res.virtual_machine_shares() << std::endl;//XXX
					if (problem_res.solver_result() == ampl::solved_result)
					{
						this->result(::dcs::eesim::detail::make_vm_placement_problem_result<traits_type>(problem_descr, problem_res));
						this->result().solved(true);
					}
				}
				break;
			case gams_optimal_solver_input_method:
				{
					namespace gams = ::dcs::eesim::detail::gams;

					// Create a new problem
					// 1. Create a problem in AMPL format
					gams::vm_placement_problem<traits_type> problem_descr;
					problem_descr = gams::make_initial_vm_placement_problem<traits_type>(dc, wp, ws, ref_penalty, vm_util_map);
					::std::string xml_job;
					xml_job = make_gams_job(xml_tmpl_,
										    problem_descr.model,
										    detail::gams_options());
					client neos;
					::std::string res;
					res = execute_job(neos, xml_job);
					gams::vm_placement_problem_result problem_res;
					problem_res = gams::make_vm_placement_problem_result(res);
::std::cerr << "solver_result: " << problem_res.solver_result() << std::endl;//XXX
::std::cerr << "model_result: " << problem_res.model_result() << std::endl;//XXX
::std::cerr << "cost: " << problem_res.cost() << std::endl;//XXX
::std::cerr << "physical_machine_selection: " << problem_res.physical_machine_selection() << std::endl;//XXX
::std::cerr << "virtual_machine_placement: " << problem_res.virtual_machine_placement() << std::endl;//XXX
::std::cerr << "virtual_machine_shares: " << problem_res.virtual_machine_shares() << std::endl;//XXX
					if (problem_res.solver_result() == gams::normal_completion_solver_result)
					{
						this->result(::dcs::eesim::detail::make_vm_placement_problem_result<traits_type>(problem_descr, problem_res));
						this->result().solved(true);
					}
				}
				break;
			default:
				throw ::std::runtime_error("[dcs::eesim::detail::neos::initial_vm_placement_minlp_solver] Input method not supported.");
		}
	}

/*
	public: bool solved() const
	{
		return problem_res_.solved;
	}


	public: real_type cost() const
	{
		return problem_res_.cost;
	}


	public: physical_virtual_machine_map const& placement() const
	{
		return problem_res_.placement;
	}
*/


	private: void init()
	{
		// Retrieve the job XML template according to the input method.
		client neos;
		xml_tmpl_ = neos.solver_template(this->category(),
										 this->solver_id(),
										 this->input_method());
	}


	/// The job XML template.
	private: ::std::string xml_tmpl_;
}; // initial_vm_placement_minlp_solver

template <typename TraitsT>
const optimal_solver_ids initial_vm_placement_minlp_solver<TraitsT>::default_solver_id(couenne_optimal_solver_id);

template <typename TraitsT>
const optimal_solver_input_methods initial_vm_placement_minlp_solver<TraitsT>::default_input_method(ampl_optimal_solver_input_method);


template <typename TraitsT>
class vm_placement_minlp_solver: public base_vm_placement_optimal_solver<TraitsT>
{
	private: typedef base_vm_placement_optimal_solver<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	private: typedef typename base_type::real_type real_type;
	private: typedef typename base_type::data_center_type data_center_type;
	private: typedef typename base_type::virtual_machine_utilization_map virtual_machine_utilization_map;


	public: static const optimal_solver_input_methods default_input_method;
	public: static const optimal_solver_ids default_solver_id;


	public: explicit vm_placement_minlp_solver(optimal_solver_ids solver_id = default_solver_id,
											   optimal_solver_input_methods method = default_input_method)
	: base_type(solver_id, method)
	{
		init();
	}


	private: optimal_solver_categories do_category() const
	{
		return minco_optimal_solver_category;
	}


	private: optimal_solver_proxies do_proxy() const
	{
		return neos_optimal_solver_proxy;
	}


    private: void do_solve(data_center_type const& dc,
						   real_type wp,
						   real_type wm,
						   real_type ws,
						   virtual_machine_utilization_map const& vm_util_map)
    {
		// Reset previous solution
		this->result().reset();

		switch (this->input_method())
		{
			case ampl_optimal_solver_input_method:
				{
					namespace ampl = ::dcs::eesim::detail::ampl;

					// Create a new problem
					// 1. Create a problem in AMPL format
					ampl::vm_placement_problem<traits_type> problem_descr;
					problem_descr = ampl::make_vm_placement_problem<traits_type>(dc, wp, wm, ws, vm_util_map);
					::std::string xml_job;
					xml_job = make_ampl_job(xml_tmpl_,
										    problem_descr.model,
										    problem_descr.data,
										    detail::ampl_options());
					client neos;
					::std::string res;
					res = execute_job(neos, xml_job);
					ampl::vm_placement_problem_result problem_res;
					problem_res = ampl::make_vm_placement_problem_result(res);
::std::cerr << "solver_exit_code: " << problem_res.solver_exit_code() << std::endl;//XXX
::std::cerr << "solver_result: " << problem_res.solver_result() << std::endl;//XXX
::std::cerr << "solver_result_code: " << problem_res.solver_result_code() << std::endl;//XXX
::std::cerr << "cost: " << problem_res.cost() << std::endl;//XXX
::std::cerr << "physical_machine_selection: " << problem_res.physical_machine_selection() << std::endl;//XXX
::std::cerr << "virtual_machine_placement: " << problem_res.virtual_machine_placement() << std::endl;//XXX
::std::cerr << "virtual_machine_shares: " << problem_res.virtual_machine_shares() << std::endl;//XXX
					if (problem_res.solver_result() == ampl::solved_result)
					{
						this->result(::dcs::eesim::detail::make_vm_placement_problem_result<traits_type>(problem_descr, problem_res));
						this->result().solved(true);
					}
				}
				break;
			case gams_optimal_solver_input_method:
				{
					namespace gams = ::dcs::eesim::detail::gams;

					// Create a new problem
					// 1. Create a problem in GAMS format
					gams::vm_placement_problem<traits_type> problem_descr;
					problem_descr = gams::make_vm_placement_problem<traits_type>(dc, wp, wm, ws, vm_util_map);
					::std::string xml_job;
					xml_job = make_gams_job(xml_tmpl_,
										    problem_descr.model,
										    detail::gams_options());
					client neos;
					::std::string res;
					res = execute_job(neos, xml_job);
					gams::vm_placement_problem_result problem_res;
					problem_res = gams::make_vm_placement_problem_result(res);
::std::cerr << "cost: " << problem_res.cost() << std::endl;//XXX
::std::cerr << "solver_result: " << problem_res.solver_result() << std::endl;//XXX
::std::cerr << "model_result: " << problem_res.model_result() << std::endl;//XXX
::std::cerr << "physical_machine_selection: " << problem_res.physical_machine_selection() << std::endl;//XXX
::std::cerr << "virtual_machine_placement: " << problem_res.virtual_machine_placement() << std::endl;//XXX
::std::cerr << "virtual_machine_shares: " << problem_res.virtual_machine_shares() << std::endl;//XXX
					if (problem_res.solver_result() == gams::normal_completion_solver_result)
					{
						this->result(::dcs::eesim::detail::make_vm_placement_problem_result<traits_type>(problem_descr, problem_res));
						this->result().solved(true);
					}
				}
				break;
			default:
				throw ::std::runtime_error("[dcs::eesim::detail::neos::vm_placement_minlp_solver] Input method not supported.");
		}
	}


	private: void init()
	{
		// Retrieve the job XML template according to the input method.
		client neos;
		xml_tmpl_ = neos.solver_template(this->category(),
										 this->solver_id(),
										 this->input_method());
	}


	private: ::std::string xml_tmpl_;
}; // vm_placement_minlp_solver

template <typename TraitsT>
const optimal_solver_ids vm_placement_minlp_solver<TraitsT>::default_solver_id(couenne_optimal_solver_id);

template <typename TraitsT>
const optimal_solver_input_methods vm_placement_minlp_solver<TraitsT>::default_input_method(ampl_optimal_solver_input_method);

}}}} // Namespace dcs::eesim::detail::neos


#endif // DCS_EESIM_DETAIL_NEOS_VM_PLACEMENT_MINLP_SOLVER_HPP
