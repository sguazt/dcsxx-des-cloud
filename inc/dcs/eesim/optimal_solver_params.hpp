#ifndef DCS_EESIM_SYSTEM_OPTIMAL_SOLVER_PARAMS_HPP
#define DCS_EESIM_SYSTEM_OPTIMAL_SOLVER_PARAMS_HPP


#include <dcs/eesim/optimal_solver_categories.hpp>
#include <dcs/eesim/optimal_solver_ids.hpp>
#include <dcs/eesim/optimal_solver_input_methods.hpp>
#include <dcs/eesim/optimal_solver_proxies.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT>
class base_optimal_solver_params
{
	public: typedef TraitsT traits_type;


	public: base_optimal_solver_params(optimal_solver_categories category,
									   optimal_solver_input_methods method,
									   optimal_solver_ids solver,
									   optimal_solver_proxies proxy = none_optimal_solver_proxy)
	: category_(category),
	  method_(method),
	  solver_(solver),
	  proxy_(proxy)
	{
	}


	public: optimal_solver_categories category() const
	{
		return category_;
	}


	public: optimal_solver_ids solver_id() const
	{
		return solver_;
	}


	public: optimal_solver_proxies proxy() const
	{
		return proxy_;
	}


	public: optimal_solver_input_methods input_method() const
	{
		return method_;
	}


	private: optimal_solver_categories category_;
	private: optimal_solver_input_methods method_;
	private: optimal_solver_ids solver_;
	private: optimal_solver_proxies proxy_;
};

template <typename TraitsT>
class optimal_solver_params: public base_optimal_solver_params<TraitsT>
{
	private: typedef base_optimal_solver_params<TraitsT> base_type;


	public: optimal_solver_params(optimal_solver_categories category,
								  optimal_solver_input_methods method,
								  optimal_solver_ids solver,
								  optimal_solver_proxies proxy)
	: base_type(category, method, solver, proxy)
	{
	}
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_SYSTEM_OPTIMAL_SOLVER_PARAMS_HPP
