/**
 * \file dcs/des/cloud/optimal_initial_placement_strategy.hpp
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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_CLOUD_OPTIMAL_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_DES_CLOUD_OPTIMAL_INITIAL_PLACEMENT_STRATEGY_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/cloud/base_initial_placement_strategy.hpp>
#include <dcs/des/cloud/data_center.hpp>
//#include <dcs/des/cloud/detail/neos/vm_placement_minlp_solver.hpp>
#include <dcs/des/cloud/detail/initial_vm_placement_optimal_solvers.hpp>
#include <dcs/des/cloud/performance_measure_category.hpp>
#include <dcs/des/cloud/virtual_machines_placement.hpp>
#include <dcs/exception.hpp>
#include <dcs/macro.hpp>
///#include <dcs/memory.hpp>
#include <map>
#include <stdexcept>
#include <vector>


namespace dcs { namespace des { namespace cloud {

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


	/// Copy constructor.
	private: optimal_initial_placement_strategy(optimal_initial_placement_strategy const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: optimal_initial_placement_strategy& operator=(optimal_initial_placement_strategy const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
	}


	private: virtual_machines_placement<traits_type> do_placement(data_center_type const& dc)
	{
DCS_DEBUG_TRACE("BEGIN Initial Placement");//XXX
::std::cerr << "[optimal_initial_placement] BEGIN Initial Placement" << ::std::endl;//XXX
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
        typedef typename data_center_type::application_type application_type;
		typedef typename application_type::reference_physical_resource_type ref_resource_type;
		typedef typename application_type::reference_physical_resource_container ref_resource_container;
		typedef typename ref_resource_container::const_iterator ref_resource_iterator;


		virtual_machine_utilization_map vm_util_map;

		// Retrieve application performance info
        {
            typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;
            typedef typename virtual_machine_container::const_iterator virtual_machine_iterator;

            virtual_machine_container vms(dc.active_virtual_machines());
            virtual_machine_iterator vm_end_it(vms.end());
            for (virtual_machine_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
            {
                virtual_machine_pointer ptr_vm(*vm_it);

				// paranoid-check: null
				DCS_DEBUG_ASSERT( ptr_vm );

				ref_resource_container rress(ptr_vm->guest_system().application().reference_resources());
				ref_resource_iterator rress_end_it(rress.end());
				for (ref_resource_iterator rress_it = rress.begin(); rress_it != rress_end_it; ++rress_it)
				{
					ref_resource_type res(*rress_it);

					//TODO: CPU resource category not yet handle in app simulation model and optimization problem
					DCS_ASSERT(
							res.category() == cpu_resource_category,
							DCS_EXCEPTION_THROW(
								::std::runtime_error,
								"Resource categories other than CPU are not yet implemented."
							)
						);

					vm_util_map[ptr_vm->id()] = ptr_vm->guest_system().application().performance_model().tier_measure(
													ptr_vm->guest_system().id(),
													::dcs::des::cloud::utilization_performance_measure
						);
::std::cerr << "[optimal_initial_placement] VM: " << *ptr_vm << " - U: " << vm_util_map.at(ptr_vm->id()) << ::std::endl;//XXX
				}
			}
		}

		// Solve the optimization problem

		//optimal_solver_type solver;

		ptr_solver_->solve(dc, wp_, ws_, this->reference_share_penalty(), vm_util_map);

		// Check solution and act accordingly

		//FIXME: handle ref_penalty

		if (!ptr_solver_->result().solved())
		{
			throw ::std::runtime_error("[dcs::des::cloud::optimal_initial_placement_strategy] Unable to solve optimal problem.");
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

::std::cerr << "[optimal_initial_placement] END Initial Placement" << ::std::endl;//XXX
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

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_OPTIMAL_INITIAL_PLACEMENT_STRATEGY_HPP
