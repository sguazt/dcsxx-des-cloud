#ifndef DCS_EESIM_BEST_FIT_DECREASING_INCREMENTAL_PLACEMENT_STRATEGY_HPP
#define DCS_EESIM_BEST_FIT_DECREASING_INCREMENTAL_PLACEMENT_STRATEGY_HPP


#include <dcs/debug.hpp>
#include <dcs/eesim/base_incremental_placement_strategy.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/detail/placement_strategy_utility.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/utility.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>
#include <map>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class best_fit_decreasing_incremental_placement_strategy: public base_incremental_placement_strategy<TraitsT>
{
	private: typedef base_incremental_placement_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	private: typedef typename base_type::virtual_machine_identifier_type vm_identifier_type;
	private: typedef typename base_type::data_center_type data_center_type;
	private: typedef typename base_type::virtual_machine_container vm_container;


	private: virtual_machines_placement<traits_type> do_place(data_center_type const& dc, vm_container const& vms)
	{
		typedef typename data_center_type::application_type application_type;
		typedef typename data_center_type::physical_machine_type pm_type;
		typedef typename data_center_type::physical_machine_pointer pm_pointer;
		typedef typename data_center_type::virtual_machine_type vm_type;
		typedef typename data_center_type::virtual_machine_pointer vm_pointer;
		typedef typename data_center_type::virtual_machines_placement_type vms_placement_type;
		typedef typename pm_type::identifier_type pm_identifier_type;
		typedef ::std::vector<pm_pointer> pm_container;
		typedef typename pm_container::const_iterator pm_iterator;
		typedef typename vm_container::const_iterator vm_iterator;
		typedef typename application_type::application_tier_type application_tier_type;
//		typedef typename application_tier_type::resource_share_type share_type;
//		typedef typename application_tier_type::resource_share_container share_container;
//		typedef typename share_container::const_iterator share_iterator;
//		typedef ::std::map<physical_resource_category,real_type> resource_share_map;
        typedef typename application_tier_type::resource_share_container ref_share_container;
        typedef ::std::map<physical_resource_category,real_type> share_container;
        typedef typename share_container::const_iterator share_iterator;
		typedef ::std::map<physical_resource_category,real_type> resource_utilization_map;
		typedef typename application_type::reference_physical_resource_type ref_resource_type;
		typedef typename application_type::reference_physical_resource_container ref_resource_container;
		typedef typename ref_resource_container::const_iterator ref_resource_iterator;

		/// Sort physical machines according to their capacity; however,
		/// powered-on machines are preferred first, then suspended, and
		/// eventually powered-off.
		/// (Sorting is done from the less powerful to the more powerful)

		//pm_container sorted_pms(dc.physical_machines());
		//::std::sort(sorted_pms.begin(),
		//			sorted_pms.end(),
		//			detail::ptr_physical_machine_less_comparator<pm_type>());
		pm_container sorted_pms;
		pm_container pms;
		pms = dc.physical_machines(powered_on_power_status);
		if (!pms.empty())
		{
			::std::sort(pms.begin(),
						pms.end(),
						detail::ptr_physical_machine_greater_by_id_comparator<pm_type>());
			::std::sort(pms.begin(),
						pms.end(),
						detail::ptr_physical_machine_greater_comparator<pm_type>());
			sorted_pms.insert(sorted_pms.end(), pms.begin(), pms.end());
		}
		pms = dc.physical_machines(suspended_power_status);
		if (!pms.empty())
		{
			::std::sort(pms.begin(),
						pms.end(),
						detail::ptr_physical_machine_greater_by_id_comparator<pm_type>());
			::std::sort(pms.begin(),
						pms.end(),
						detail::ptr_physical_machine_greater_comparator<pm_type>());
			sorted_pms.insert(sorted_pms.end(), pms.begin(), pms.end());
		}
		pms = dc.physical_machines(powered_off_power_status);
		if (!pms.empty())
		{
			::std::sort(pms.begin(),
						pms.end(),
						detail::ptr_physical_machine_greater_by_id_comparator<pm_type>());
			::std::sort(pms.begin(),
						pms.end(),
						detail::ptr_physical_machine_greater_comparator<pm_type>());
			sorted_pms.insert(sorted_pms.end(), pms.begin(), pms.end());
		}
		pms.clear();

		// Sort virtual machines according to their shares
		vm_container sorted_vms(vms);
		::std::sort(sorted_vms.begin(),
					sorted_vms.end(),
					detail::ptr_virtual_machine_greater_by_id_comparator<vm_type>());
		::std::sort(sorted_vms.begin(),
					sorted_vms.end(),
					detail::ptr_virtual_machine_greater_comparator<vm_type>());

DCS_DEBUG_TRACE("BEGIN Incremental Placement");//XXX
::std::cerr << "[best_fit_decreasing_incremental_placement] BEGIN Incremental Placement" << ::std::endl;//XXX
DCS_DEBUG_TRACE("#Machines: " << sorted_pms.size());//XXX
DCS_DEBUG_TRACE("#VMs: " << sorted_vms.size());//XXX

		vms_placement_type deployment(dc.current_virtual_machines_placement());

		vm_iterator vm_end_it(sorted_vms.end());
		for (vm_iterator vm_it = sorted_vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			vm_pointer ptr_vm(*vm_it);

			// paranoid-check: valid pointer
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
			pm_iterator pm_end_it(sorted_pms.end());
			share_iterator ref_share_end_it(ref_shares.end());
			for (pm_iterator pm_it = sorted_pms.begin(); pm_it != pm_end_it && !placed; ++pm_it)
			{
				pm_pointer ptr_pm(*pm_it);

				// paranoid-check: valid pointer
				DCS_DEBUG_ASSERT( ptr_pm );

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
					share = ::dcs::eesim::scale_resource_share(ref_capacity,
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
DCS_DEBUG_TRACE("Placed: VM(" << ptr_vm->id() << ") -> PM(" << ptr_pm->id() << ") ==> OK? " <<  std::boolalpha << placed);///XXX
::std::cerr << "[best_fit_decreasing_incremental_placement] Placed: VM(" << ptr_vm->id() << ") -> PM(" << ptr_pm->id() << ") with SHARE: " << shares.at(cpu_resource_category) << " ==> OK? " <<  std::boolalpha << placed << ::std::endl;///XXX
			}

			if (!placed)
			{
				// Choose the PM with the largest free space
				typedef typename vms_placement_type::const_iterator vmp_iterator;
				typedef typename vms_placement_type::share_const_iterator vmp_share_iterator;
				typedef ::std::map<pm_identifier_type,share_container> pm_share_map;
				typedef typename pm_share_map::const_iterator pm_share_iterator;
				//typedef typename resource_share_map::const_iterator resource_share_iterator;

				pm_share_map pm_shares;

				// Compute the free share for each PM
				vmp_iterator vmp_end_it(deployment.end());
				for (vmp_iterator vmp_it = deployment.begin(); vmp_it != vmp_end_it; ++vmp_it)
				{
					pm_identifier_type pm_id(deployment.pm_id(vmp_it));

					vmp_share_iterator share_end_it(deployment.shares_end(vmp_it));
					for (vmp_share_iterator share_it = deployment.shares_begin(vmp_it); share_it != share_end_it; ++share_it)
					{
						physical_resource_category category(share_it->first);

						if (pm_shares.count(pm_id) && pm_shares.at(pm_id).count(category))
						{
							pm_shares[pm_id][category] -= share_it->second;
						}
						else
						{
							pm_shares[pm_id][category] = 1-share_it->second;
						}
					}
				}

				// Find the PM with the max free share
				pm_identifier_type max_pm_id;
				share_container max_pm_shares;
				pm_share_iterator pm_share_end_it(pm_shares.end());
				for (pm_share_iterator pm_share_it = pm_shares.begin(); pm_share_it != pm_share_end_it; ++pm_share_it)
				{
					pm_identifier_type pm_id(pm_share_it->first);

					share_iterator share_end_it(pm_share_it->second.end());
					for (share_iterator share_it = pm_share_it->second.begin(); share_it != share_end_it; ++share_it)
					{
						physical_resource_category category(share_it->first);
						real_type free_share(share_it->second);

						//TODO: CPU resource category not yet handle in app simulation model
						DCS_ASSERT(
								category == cpu_resource_category,
								DCS_EXCEPTION_THROW(
									::std::runtime_error,
									"Resource categories other than CPU are not yet implemented."
								)
							);

						if (free_share > 0 && (!max_pm_shares.count(category) || max_pm_shares.at(category) < free_share))
						{
							max_pm_shares[category] = free_share;
							max_pm_id = pm_id;
						}
					}
				}

				if (max_pm_shares.size() > 0)
				{
					pm_pointer ptr_pm(dc.physical_machine_ptr(max_pm_id));

					// paranoid-check: null
					DCS_DEBUG_ASSERT( ptr_pm );

					// Try to place current VM on current PM
					placed = deployment.try_place(*ptr_vm,
												  *ptr_pm,
												  max_pm_shares.begin(),
												  max_pm_shares.end());
	DCS_DEBUG_TRACE("Placed: VM(" << ptr_vm->id() << ") -> PM(" << ptr_pm->id() << ") ==> OK? " <<  std::boolalpha << placed);///XXX
	::std::cerr << "[best_fit_decreasing_incremental_placement] Placed: VM(" << ptr_vm->id() << ") -> PM(" << ptr_pm->id() << ") with SHARE: " << max_pm_shares.at(cpu_resource_category) << " ==> OK? " <<  std::boolalpha << placed << ::std::endl;///XXX
				}
			}
		}

::std::cerr << "[best_fit_decreasing_incremental_placement] END Incremental Placement" << ::std::endl;//XXX
DCS_DEBUG_TRACE("END Incremental Placement ==> " << deployment);///XXX
		return deployment;
	}
}; // best_fit_decreasing_incremental_placement_strategy

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BEST_FIT_DECREASING_INCREMENTAL_PLACEMENT_STRATEGY_HPP
