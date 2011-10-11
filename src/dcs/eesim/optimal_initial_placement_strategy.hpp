/**
 * \file src/dcs/eesim/optimal_initial_placement_strategy.hpp
 *
 * \brief Optimal initial placement strategy based on mathematical programming. 
 *
 * Copyright (C) 2009-2011  Distributed Computing System (DCS) Group, Computer
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
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_EESIM_OPTIMAL_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_EESIM_OPTIMAL_INITIAL_PLACEMENT_STRATEGY_HPP


#include <dcs/debug.hpp>
#include <dcs/eesim/base_initial_placement_strategy.hpp>
#include <dcs/eesim/data_center.hpp>
//#include <dcs/eesim/detail/neos/vm_placement_minlp_solver.hpp>
#include <dcs/eesim/detail/initial_vm_placement_optimal_solvers.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>
///#include <dcs/memory.hpp>
#include <map>
#include <stdexcept>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class optimal_initial_placement_strategy: public base_initial_placement_strategy<TraitsT>
{
	private: typedef base_initial_placement_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef data_center<traits_type> data_center_type;
	public: typedef base_optimal_solver_params<traits_type> optimal_solver_params_type;
	//private: typedef detail::neos::initial_vm_placement_minlp_solver<traits_type> optimal_solver_type;
	private: typedef detail::base_initial_vm_placement_optimal_solver<traits_type> optimal_solver_type;
	private: typedef ::dcs::shared_ptr<optimal_solver_type> optimal_solver_pointer;
	//public: typedef ::dcs::shared_ptr<optimal_solver_params_type> optimal_solver_params_pointer;


	public: static const real_type default_power_cost_weight;
	public: static const real_type default_sla_cost_weight;


	public: explicit optimal_initial_placement_strategy(optimal_solver_params_type const& solver_params,
														real_type wp = default_power_cost_weight,
														real_type ws = default_sla_cost_weight,
														real_type ref_penalty = base_type::default_reference_share_penalty)
	: base_type(ref_penalty),
	  wp_(wp),
	  ws_(ws),
	  ptr_solver_(detail::make_initial_vm_placement_optimal_solver(solver_params))
	{
	}


	private: virtual_machines_placement<traits_type> do_placement(data_center_type const& dc)
	{
DCS_DEBUG_TRACE("BEGIN Initial Placement");//XXX
		typedef typename traits_type::physical_machine_identifier_type physical_machine_identifier_type;
		typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
		typedef ::std::map<virtual_machine_identifier_type,real_type> virtual_machine_utilization_map;
		typedef typename optimal_solver_type::physical_virtual_machine_map physical_virtual_machine_map;
		typedef typename physical_virtual_machine_map::const_iterator physical_virtual_machine_iterator;
		typedef typename optimal_solver_type::resource_share_container resource_share_container;
		typedef typename data_center_type::physical_machine_type physical_machine_type;
		typedef typename data_center_type::physical_machine_pointer physical_machine_pointer;
		typedef typename data_center_type::virtual_machine_type virtual_machine_type;
		typedef typename data_center_type::virtual_machine_pointer virtual_machine_pointer;


		virtual_machine_utilization_map vm_util_map;

		// Retrieve application performance info
        {
            typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;
            typedef typename virtual_machine_container::const_iterator virtual_machine_iterator;

            virtual_machine_container vms(dc.virtual_machines());
            virtual_machine_iterator vm_end_it(vms.end());
            for (virtual_machine_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
            {
                virtual_machine_pointer ptr_vm(*vm_it);

				vm_util_map[ptr_vm->id()] = ptr_vm->guest_system().application().performance_model().tier_measure(
												ptr_vm->guest_system().id(),
												::dcs::eesim::utilization_performance_measure
					);
			}
		}

		// Solve the optimization problem

		//optimal_solver_type solver;

		ptr_solver_->solve(dc, wp_, ws_, this->reference_share_penalty(), vm_util_map);

		// Check solution and act accordingly

		//FIXME: handle ref_penalty

		if (!ptr_solver_->result().solved())
		{
			throw ::std::runtime_error("[dcs::eesim::optimal_initial_placement_strategy] Unable to solve optimal problem.");
		}

		virtual_machines_placement<traits_type> deployment;

		physical_virtual_machine_map pm_vm_map(ptr_solver_->result().placement());
//[XXX]
::std::cerr << "CHECK SOLVER INITIAL PLACEMENT" << ::std::endl;//XXX
for (typename physical_virtual_machine_map::const_iterator it = ptr_solver_->result().placement().begin(); it != ptr_solver_->result().placement().end(); ++it)//XXX
{//XXX
::std::cerr << "VM ID: " << (it->first.second) << " placed on PM ID: " << (it->first.first) << " with SHARE: " << ((it->second)[0].second) << ::std::endl;//XXX
}//XXX
//[/XXX]
		physical_virtual_machine_iterator pm_vm_end_it(pm_vm_map.end());
		for (physical_virtual_machine_iterator pm_vm_it = pm_vm_map.begin(); pm_vm_it != pm_vm_end_it; ++pm_vm_it)
		{
			physical_machine_pointer ptr_pm(dc.physical_machine_ptr(pm_vm_it->first.first));
			virtual_machine_pointer ptr_vm(dc.virtual_machine_ptr(pm_vm_it->first.second));
			resource_share_container shares(pm_vm_it->second);

//::std::cerr << "Going to migrate VM (" << pm_vm_it->first.second << "): " << *ptr_vm << " into PM (" << pm_vm_it->first.first << "): " << *ptr_pm << ::std::endl; //XXX
			deployment.place(*ptr_vm,
							 *ptr_pm,
							 shares.begin(),
							 shares.end());
DCS_DEBUG_TRACE("Placed: VM(" << ptr_vm->id() << ") -> PM(" << ptr_pm->id() << ")");//XXX
		}

DCS_DEBUG_TRACE("END Initial Placement ==> " << deployment);///XXX
		return deployment;
	}


	/// Weight for power consumption cost.
	private: real_type wp_;
	/// Weight for resource share cost.
	private: real_type ws_;
	private: optimal_solver_pointer ptr_solver_;
}; // optimal_initial_placement_strategy

template <typename TraitsT>
const typename TraitsT::real_type optimal_initial_placement_strategy<TraitsT>::default_power_cost_weight(1);

template <typename TraitsT>
const typename TraitsT::real_type optimal_initial_placement_strategy<TraitsT>::default_sla_cost_weight(1);

}} // Namespace dcs::eesim


#endif // DCS_EESIM_OPTIMAL_INITIAL_PLACEMENT_STRATEGY_HPP
