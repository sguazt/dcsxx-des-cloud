/**
 * \file dcs/des/cloud/first_fit_scaleout_initial_placement_strategy.hpp
 *
 * \brief Initial VM placement based a FIRST-FIT scale-out strategy.
 *
 * Copyright (C) 2009-2010  Distributed Computing System (DCS) Group, Computer
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

#ifndef DCS_DES_CLOUD_FIRST_FIT_SCALEOUT_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_DES_CLOUD_FIRST_FIT_SCALEOUT_INITIAL_PLACEMENT_STRATEGY_HPP


#include <cstddef>
#include <dcs/debug.hpp>
#include <dcs/des/cloud/base_initial_placement_strategy.hpp>
#include <dcs/des/cloud/data_center.hpp>
#include <dcs/des/cloud/performance_measure_category.hpp>
#include <dcs/des/cloud/physical_resource_category.hpp>
#include <dcs/des/cloud/utility.hpp>
#include <dcs/des/cloud/virtual_machines_placement.hpp>
#include <map>
#include <set>
#include <vector>


namespace dcs { namespace des { namespace cloud {

/// Assign at most one virtual machine to each physical machine.
template <typename TraitsT>
class first_fit_scaleout_initial_placement_strategy: public base_initial_placement_strategy<TraitsT>
{
	public: typedef TraitsT traits_type;
	private: typedef typename traits_type::real_type real_type;
	private: typedef data_center<traits_type> data_center_type;


	private: virtual_machines_placement<traits_type> do_placement(data_center_type const& dc)
	{
		typedef typename data_center_type::application_type application_type;
		typedef typename data_center_type::physical_machine_type pm_type;
		typedef typename data_center_type::physical_machine_pointer pm_pointer;
		typedef typename data_center_type::virtual_machine_type vm_type;
		typedef typename data_center_type::virtual_machine_pointer vm_pointer;
		typedef typename pm_type::identifier_type pm_identifier_type;
		typedef typename vm_type::identifier_type vm_identifier_type;
		typedef ::std::vector<pm_pointer> pm_container;
		typedef ::std::vector<vm_pointer> vm_container;
		typedef typename pm_container::const_iterator pm_iterator;
		typedef typename vm_container::const_iterator vm_iterator;
		typedef ::std::size_t size_type;
//		typedef ::std::pair<physical_resource_category,real_type> share_type;
//		typedef ::std::vector<share_type> share_container;
//		typedef typename share_container::const_iterator share_iterator;
//		typedef ::std::map<physical_resource_category,real_type> resource_share_map;
		typedef typename application_type::application_tier_type application_tier_type;
		typedef typename application_tier_type::resource_share_container ref_share_container;
		typedef ::std::map<physical_resource_category,real_type> share_container;
		typedef typename share_container::const_iterator share_iterator;
		typedef ::std::map<physical_resource_category,real_type> resource_utilization_map;
		typedef typename application_type::reference_physical_resource_type ref_resource_type;
		typedef typename application_type::reference_physical_resource_container ref_resource_container;
		typedef typename ref_resource_container::const_iterator ref_resource_iterator;

		vm_container vms(dc.active_virtual_machines());

		pm_container pms(dc.physical_machines());

		size_type npms(pms.size());
		size_type nvms(vms.size());

DCS_DEBUG_TRACE("BEGIN Initial Placement");//XXX
DCS_DEBUG_TRACE("#Machines: " << npms);//XXX
DCS_DEBUG_TRACE("#VMs: " << nvms);//XXX

		if (nvms > npms)
		{
			throw ::std::logic_error("[dcs::des::cloud::first_fit_scaleout_initial_placement_strategy] Too many virtual machines.");
		}

		virtual_machines_placement<traits_type> deployment;

		::std::vector< ::std::pair<physical_resource_category,real_type> > shares;
		::std::set<pm_identifier_type> used_pms;
		vm_iterator vm_end_it(vms.end());
		for (vm_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			vm_pointer ptr_vm(*vm_it);

			// paranoid-check: valid pointer.
			DCS_DEBUG_ASSERT( ptr_vm );

			application_type const& app(ptr_vm->guest_system().application());

			share_container ref_shares;
			{
				ref_share_container tmp_shares(ptr_vm->guest_system().resource_shares());
				ref_shares = share_container(tmp_shares.begin(), tmp_shares.end());
			}

			// Retrieve the share for every resource of the VM guest system
			//share_container ref_shares(ptr_vm->guest_system().resource_shares());
			ref_resource_container rress(app.reference_resources());
			ref_resource_iterator rress_end_it(rress.end());
 
			// For each physical machine PM, try to deploy current VM on PM
			// until a suitable machine (i.e., a machine with sufficient free
			// capacity) is found.
			bool placed(false);
			pm_pointer ptr_pm;
			pm_iterator pm_end_it(pms.end());
			share_iterator ref_share_end_it(ref_shares.end());
			for (pm_iterator pm_it = pms.begin(); pm_it != pm_end_it && !placed; ++pm_it)
			{
				ptr_pm = *pm_it;

				// paranoid-check: valid pointer.
				DCS_DEBUG_ASSERT( ptr_pm );

				if (used_pms.count(ptr_pm->id()))
				{
					// Machine already occupied
					continue;
				}

				// Reference to actual resource shares
				share_container shares;
				for (share_iterator ref_share_it = ref_shares.begin(); ref_share_it != ref_share_end_it; ++ref_share_it)
				{
					physical_resource_category ref_category(ref_share_it->first);
					real_type ref_share(ref_share_it->second);
					if (this->reference_share_penalty() > 0)
					{
						ref_share -= ref_share*this->reference_share_penalty();
					}
					real_type ref_capacity(app.reference_resource(ref_category).capacity());
					//real_type ref_threshold(app.reference_resource(ref_category).utilization_threshold());

					real_type actual_capacity(ptr_pm->resource(ref_category)->capacity());
					//real_type actual_threshold(ptr_pm->resource(ref_category)->utilization_threshold());

					real_type share;

					// Scale share in terms of actual machine
					share = scale_resource_share(ref_capacity,
												 //ref_threshold,
												 actual_capacity,
												 //actual_threshold,
												 ref_share);

					shares[ref_category] = share;
				}

				// Reference to actual resource utilization
				resource_utilization_map utils;
				for (ref_resource_iterator rress_it = rress.begin(); rress_it != rress_end_it; ++rress_it)
				{
					ref_resource_type res(*rress_it);

					 //TODO: CPU resource category not yet handle in app simulation model
					DCS_ASSERT(
							res.category() == cpu_resource_category,
							DCS_EXCEPTION_THROW(
								::std::runtime_error,
								"Resource categories other than CPU are not yet implemented."
							)
						);

					// paranoid-check: consistency
					DCS_DEBUG_ASSERT( ref_shares.count(res.category()) > 0 );

					// paranoid-check: consistency
					DCS_DEBUG_ASSERT( shares.count(res.category()) > 0 );

					real_type util(0);
					util = app.performance_model().tier_measure(
									ptr_vm->guest_system().id(),
									utilization_performance_measure
						);

					// Scale utilization in terms of actual machine
					util = scale_resource_utilization(res.capacity(),
													  ref_shares.at(res.category()),
													  ptr_pm->resource(res.category())->capacity(),
													  shares.at(res.category()),
													  util,
													  ptr_pm->resource(res.category())->utilization_threshold());

					utils[res.category()] = util;
				}

				// Try to place current VM on current PM
                placed = deployment.try_place(*ptr_vm,
                                              *ptr_pm,
                                              shares.begin(),
                                              shares.end(),
											  utils.begin(),
											  utils.end(),
											  dc);
			}
DCS_DEBUG_TRACE("Placed: VM(" << ptr_vm->id() << ") -> PM(" << ptr_pm->id() << ") ==> OK? " <<  std::boolalpha << placed);///XXX

			if (placed)
			{
//				pms.erase(ptr_pm);
				used_pms.insert(ptr_pm->id());
			}
		}

DCS_DEBUG_TRACE("END Initial Placement ==> " << deployment);///XXX
		return deployment;
	}
}; // first_fit_scaleout_initial_placement_strategy

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_FIRST_FIT_SCALEOUT_INITIAL_PLACEMENT_STRATEGY_HPP
