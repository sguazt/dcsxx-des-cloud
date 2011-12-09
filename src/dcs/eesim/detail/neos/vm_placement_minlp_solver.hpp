#ifndef DCS_EESIM_DETAIL_NEOS_VM_PLACEMENT_MINLP_SOLVER_HPP
#define DCS_EESIM_DETAIL_NEOS_VM_PLACEMENT_MINLP_SOLVER_HPP


#include <dcs/eesim/best_fit_decreasing_initial_placement_strategy.hpp>
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
#include <dcs/eesim/logging.hpp>
#include <dcs/eesim/optimal_solver_categories.hpp>
#include <dcs/eesim/optimal_solver_ids.hpp>
#include <dcs/eesim/optimal_solver_input_methods.hpp>
#include <dcs/eesim/optimal_solver_proxies.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>
#include <exception>
#include <string>
#include <unistd.h>


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
			"if solve_result_num < 0 then {"
			" for {i in I} {"
			"  let shares_sum := sum{j in J} round(s[i,j],5);"
			"  if shares_sum > Smax[i] then"
			"   let{j in J} s[i,j] := round(s[i,j]*Smax[i]/shares_sum,5);"
			"  else"
			"   let{j in J} s[i,j] := round(s[i,j],5);"
			" }"
			"}"
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
//	return	"option sysout = on;";
	return "";
}

}} // Namespace detail::<unnamed>


template <typename TraitsT>
class initial_vm_placement_minlp_solver: public base_initial_vm_placement_optimal_solver<TraitsT>
{
	private: typedef base_initial_vm_placement_optimal_solver<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	private: typedef typename traits_type::uint_type uint_type;
	private: typedef typename traits_type::real_type real_type;
	private: typedef typename base_type::data_center_type data_center_type;
	private: typedef typename base_type::virtual_machine_utilization_map virtual_machine_utilization_map;


	public: static const optimal_solver_input_methods default_input_method;
	public: static const optimal_solver_ids default_solver_id;
	private: static const uint_type default_max_num_fails;
	private: static const real_type default_initial_sleep_time;


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

		// As initial guess use a specif heuristic
		//FIXME: Best-Fit-Decreasing heuristic is hard-coded
		best_fit_decreasing_initial_placement_strategy<traits_type> heuristic_strategy;
		heuristic_strategy.reference_share_penalty(ref_penalty);
		virtual_machines_placement<traits_type> init_guess(heuristic_strategy.placement(dc));

		switch (this->input_method())
		{
			case ampl_optimal_solver_input_method:
				{
					namespace ampl = ::dcs::eesim::detail::ampl;

					// Create a new problem
					// 1. Create a problem in AMPL format
					ampl::vm_placement_problem<traits_type> problem_descr;
					problem_descr = ampl::make_initial_vm_placement_problem<traits_type>(dc, wp, ws, ref_penalty, vm_util_map, init_guess);
::std::cerr << "Created AMPL problem: " << problem_descr.model << "reset data; data;" << problem_descr.data << ::std::endl;//XXX
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

					bool ok(false);
					uint_type num_fails(0);
					real_type zzz_time(default_initial_sleep_time);
					while (!ok)
					{
						try
						{
							res = execute_job(neos, xml_job);
							ok = true;
						}
						catch (::std::exception const& ex)
						{
							++num_fails;
							if (num_fails == default_max_num_fails)
							{
								//throw ex;
								log_warn(DCS_EESIM_LOGGING_AT, ex.what());
								return;
							}
							::sleep(zzz_time);
							zzz_time *= 1.5; // exponential backoff (1.5 -> 50% increase per back off)
						}
					}

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
					problem_descr = gams::make_initial_vm_placement_problem<traits_type>(dc, wp, ws, ref_penalty, vm_util_map, init_guess);
					::std::string xml_job;
::std::cerr << "Created GAMS problem: " << problem_descr.model << ::std::endl;//XXX
					xml_job = make_gams_job(xml_tmpl_,
										    problem_descr.model,
										    detail::gams_options());
					client neos;
					::std::string res;

					bool ok(false);
					uint_type num_fails(0);
					real_type zzz_time(default_initial_sleep_time);
//					while (!ok)
					do
					{
						try
						{
							res = execute_job(neos, xml_job);
							ok = true;
						}
						catch (::std::exception const& ex)
						{
							++num_fails;
::std::cerr << "Waiting... (Failure: " << num_fails << "/" << default_max_num_fails << ", Zzz: " << zzz_time << ")" << ::std::endl;//XXX
							log_warn(DCS_EESIM_LOGGING_AT, ex.what());
//							if (num_fails == default_max_num_fails)
//							{
//								//throw ex;
//								log_warn(DCS_EESIM_LOGGING_AT, ex.what());
//								return;
//							}
							::sleep(zzz_time);
							zzz_time *= 1.5; // exponential backoff (1.5 -> 50% increase per back off)
						}
					}
					while (!ok && num_fails <= default_max_num_fails);

					if (!ok)
					{
						return;
					}

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
const typename TraitsT::uint_type initial_vm_placement_minlp_solver<TraitsT>::default_max_num_fails(10);

template <typename TraitsT>
const typename TraitsT::real_type initial_vm_placement_minlp_solver<TraitsT>::default_initial_sleep_time(5);


template <typename TraitsT>
class vm_placement_minlp_solver: public base_vm_placement_optimal_solver<TraitsT>
{
	private: typedef base_vm_placement_optimal_solver<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	private: typedef typename traits_type::uint_type uint_type;
	private: typedef typename traits_type::real_type real_type;
	private: typedef typename base_type::data_center_type data_center_type;
	private: typedef typename base_type::virtual_machine_utilization_map virtual_machine_utilization_map;
	private: typedef typename base_type::virtual_machine_share_map virtual_machine_share_map;


	public: static const optimal_solver_input_methods default_input_method;
	public: static const optimal_solver_ids default_solver_id;
	private: static const uint_type default_max_num_fails;
	private: static const real_type default_initial_sleep_time;


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
						   virtual_machine_utilization_map const& vm_util_map,
						   virtual_machine_share_map const& vm_share_map)
    {
		// Reset previous solution
		this->result().reset();

		// As initial guess use the current VM placement.
		virtual_machines_placement<traits_type> init_guess(dc.current_virtual_machines_placement());

		switch (this->input_method())
		{
			case ampl_optimal_solver_input_method:
				{
					namespace ampl = ::dcs::eesim::detail::ampl;

					// Create a new problem
					// 1. Create a problem in AMPL format
					ampl::vm_placement_problem<traits_type> problem_descr;
					problem_descr = ampl::make_vm_placement_problem<traits_type>(dc,
																				 wp,
																				 wm,
																				 ws,
																				 vm_util_map.begin(),
																				 vm_util_map.end(),
																				 vm_share_map.begin(),
																				 vm_share_map.end(),
																				 init_guess);
::std::cerr << "Created AMPL problem: " << problem_descr.model << "reset data; data;" << problem_descr.data << ::std::endl;//XXX
					::std::string xml_job;
					xml_job = make_ampl_job(xml_tmpl_,
										    problem_descr.model,
										    problem_descr.data,
										    detail::ampl_options());
					client neos;
					::std::string res;

					bool ok(false);
					uint_type num_fails(0);
					real_type zzz_time(default_initial_sleep_time);
					while (!ok)
					{
						try
						{
							res = execute_job(neos, xml_job);
							ok = true;
						}
						catch (::std::exception const& ex)
						{
							++num_fails;
							if (num_fails == default_max_num_fails)
							{
								//throw ex;
								log_warn(DCS_EESIM_LOGGING_AT, ex.what());
								return;
							}
							::sleep(zzz_time);
							zzz_time *= 1.5; // exponential backoff (1.5 -> 50% increase per back off)
						}
					}

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
					problem_descr = gams::make_vm_placement_problem<traits_type>(dc,
																				 wp,
																				 wm,
																				 ws,
																				 vm_util_map.begin(),
																				 vm_util_map.end(),
																				 vm_share_map.begin(),
																				 vm_share_map.end(),
																				 init_guess);
::std::cerr << "Created GAMS problem: " << problem_descr.model << ::std::endl;//XXX
					::std::string xml_job;
					xml_job = make_gams_job(xml_tmpl_,
										    problem_descr.model,
										    detail::gams_options());
					client neos;
					::std::string res;

					bool ok(false);
					uint_type num_fails(0);
					real_type zzz_time(default_initial_sleep_time);
					do
					{
						try
						{
							res = execute_job(neos, xml_job);
							ok = true;
						}
						catch (::std::exception const& ex)
						{
							++num_fails;
::std::cerr << "Waiting... (Failure: " << num_fails << "/" << default_max_num_fails << ", Zzz: " << zzz_time << ")" << ::std::endl;//XXX
							log_warn(DCS_EESIM_LOGGING_AT, ex.what());
//							if (num_fails == default_max_num_fails)
//							{
//								//throw ex;
//								return;
//							}
							::sleep(zzz_time);
							zzz_time *= 1.5; // exponential backoff (1.5 -> 50% increase per back off)
						}
					}
					while (!ok && num_fails <= default_max_num_fails);

					if (!ok)
					{
						return;
					}

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

template <typename TraitsT>
const typename TraitsT::uint_type vm_placement_minlp_solver<TraitsT>::default_max_num_fails(10);

template <typename TraitsT>
const typename TraitsT::real_type vm_placement_minlp_solver<TraitsT>::default_initial_sleep_time(5);

}}}} // Namespace dcs::eesim::detail::neos


#endif // DCS_EESIM_DETAIL_NEOS_VM_PLACEMENT_MINLP_SOLVER_HPP
