#ifndef DCS_EESIM_DETAIL_BASE_VM_PLACEMENT_OPTIMAL_SOLVER_HPP
#define DCS_EESIM_DETAIL_BASE_VM_PLACEMENT_OPTIMAL_SOLVER_HPP


#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/optimal_solver_categories.hpp>
#include <dcs/eesim/optimal_solver_ids.hpp>
#include <dcs/eesim/optimal_solver_input_methods.hpp>
#include <dcs/eesim/optimal_solver_proxies.hpp>
#include <dcs/eesim/detail/vm_placement_problem.hpp>
#include <map>


namespace dcs { namespace eesim { namespace detail {

template <typename TraitsT>
class base_vm_placement_optimal_solver
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef data_center<traits_type> data_center_type;
	public: typedef typename traits_type::physical_machine_identifier_type physical_machine_identifier_type;
	public: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
	public: typedef ::std::map<virtual_machine_identifier_type,real_type> virtual_machine_utilization_map;
	private: typedef vm_placement_problem_result<traits_type> problem_result_type;
	public: typedef typename problem_result_type::physical_virtual_machine_pair_type physical_virtual_machine_pair_type;
	public: typedef typename problem_result_type::resource_share_container resource_share_container;
	public: typedef typename problem_result_type::physical_virtual_machine_map physical_virtual_machine_map;


	public: base_vm_placement_optimal_solver()
	{
	}


	public: base_vm_placement_optimal_solver(optimal_solver_ids id,
											 optimal_solver_input_methods method)
	: sid_(id),
	  method_(method)
	{
	}


	public: optimal_solver_ids solver_id() const
	{
		return sid_;
	}


	public: optimal_solver_input_methods input_method() const
	{
		return method_;
	}


	public: optimal_solver_categories category() const
	{
		return do_category();
	}


	public: optimal_solver_proxies proxy() const
	{
		return do_proxy();
	}


	public: void solve(data_center_type const& dc,
					   real_type wp,
					   real_type wm,
					   real_type ws,
					   virtual_machine_utilization_map const& vm_util_map)
	{
		do_solve(dc, wp, wm, ws, vm_util_map);
	}


	public: problem_result_type const& result() const
	{
		return result_;
	}


	public: problem_result_type& result()
	{
		return result_;
	}


	protected: void result(problem_result_type const& result)
	{
		result_ = result;
	}


	private: virtual optimal_solver_categories do_category() const = 0;


	private: virtual optimal_solver_proxies do_proxy() const = 0;


	private: virtual void do_solve(data_center_type const& dc,
								   real_type wp,
								   real_type wm,
								   real_type ws,
								   virtual_machine_utilization_map const& vm_util_map) = 0;


	private: optimal_solver_ids sid_;
	private: optimal_solver_input_methods method_;
	private: problem_result_type result_;
};

}}} // Namespace dcs::eesim::detail


#endif // DCS_EESIM_DETAIL_BASE_VM_PLACEMENT_OPTIMAL_SOLVER_HPP
