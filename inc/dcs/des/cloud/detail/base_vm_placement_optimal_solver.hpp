/**
 * \file dcs/des/cloud/detail/base_vm_placement_optimal_solver.hpp
 *
 * \brief Base class for VM placement based on optimal strategies.
 *
 * Copyright (C) 2009-2012  Distributed Computing System (DCS) Group, Computer
 * Science Department - University of Piemonte Orientale, Alessandria (Italy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_CLOUD_DETAIL_BASE_VM_PLACEMENT_OPTIMAL_SOLVER_HPP
#define DCS_DES_CLOUD_DETAIL_BASE_VM_PLACEMENT_OPTIMAL_SOLVER_HPP


#include <dcs/des/cloud/data_center.hpp>
#include <dcs/des/cloud/optimal_solver_categories.hpp>
#include <dcs/des/cloud/optimal_solver_ids.hpp>
#include <dcs/des/cloud/optimal_solver_input_methods.hpp>
#include <dcs/des/cloud/optimal_solver_proxies.hpp>
#include <dcs/des/cloud/detail/vm_placement_problem.hpp>
#include <map>


namespace dcs { namespace des { namespace cloud { namespace detail {

template <typename TraitsT>
class base_vm_placement_optimal_solver
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef data_center<traits_type> data_center_type;
	public: typedef typename traits_type::physical_machine_identifier_type physical_machine_identifier_type;
	public: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
	public: typedef ::std::map<virtual_machine_identifier_type,real_type> virtual_machine_utilization_map;
	public: typedef vm_placement_problem_result<traits_type> problem_result_type;
//	protected: typedef typename problem_result_type::physical_virtual_machine_pair_type physical_virtual_machine_pair_type;
	protected: typedef typename problem_result_type::resource_share_container resource_share_container;
//	protected: typedef typename problem_result_type::physical_virtual_machine_map physical_virtual_machine_map;
	protected: typedef ::std::map<virtual_machine_identifier_type,resource_share_container> virtual_machine_share_map;


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


//	public: void solve(data_center_type const& dc,
//					   real_type wp,
//					   real_type wm,
//					   real_type ws,
//					   virtual_machine_utilization_map const& vm_util_map)
//	{
//	}

	public: template <typename UtilFwdIterT, typename ShareFwdIterT>
			void solve(data_center_type const& dc,
					   real_type wp,
					   real_type wm,
					   real_type ws,
					   UtilFwdIterT vm_util_first,
					   UtilFwdIterT vm_util_last,
					   ShareFwdIterT vm_share_first,
					   ShareFwdIterT vm_share_last)
	{
		virtual_machine_utilization_map vm_util_map(vm_util_first, vm_util_last);

		virtual_machine_share_map vm_share_map;
		while (vm_share_first != vm_share_last)
		{
			vm_share_map[vm_share_first->first] = resource_share_container(vm_share_first->second.begin(),
																		   vm_share_first->second.end());
			++vm_share_first;
		}

		do_solve(dc, wp, wm, ws, vm_util_map, vm_share_map);
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
								   virtual_machine_utilization_map const& vm_util_map,
								   virtual_machine_share_map const& vm_share_map) = 0;


	private: optimal_solver_ids sid_;
	private: optimal_solver_input_methods method_;
	private: problem_result_type result_;
};

}}}} // Namespace dcs::des::cloud::detail


#endif // DCS_DES_CLOUD_DETAIL_BASE_VM_PLACEMENT_OPTIMAL_SOLVER_HPP
