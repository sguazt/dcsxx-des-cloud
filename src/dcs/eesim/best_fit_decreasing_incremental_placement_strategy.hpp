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
		typedef typename pm_type::identifier_type pm_identifier_type;
		typedef typename data_center_type::virtual_machine_type vm_type;
		typedef typename data_center_type::virtual_machine_pointer vm_pointer;
		typedef ::std::vector<pm_pointer> pm_container;
		typedef typename pm_container::const_iterator pm_iterator;
		typedef typename vm_container::const_iterator vm_iterator;
		typedef ::std::pair<physical_resource_category,real_type> share_type;
		typedef ::std::vector<share_type> share_container;
		typedef typename share_container::const_iterator share_iterator;
		typedef ::std::map<physical_resource_category,real_type> resource_share_map;
		typedef ::std::map<physical_resource_category,real_type> resource_utilization_map;

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
						detail::ptr_physical_machine_greater_comparator<pm_type>());
			sorted_pms.insert(sorted_pms.end(), pms.begin(), pms.end());
		}
		pms = dc.physical_machines(suspended_power_status);
		if (!pms.empty())
		{
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
						detail::ptr_physical_machine_greater_comparator<pm_type>());
			sorted_pms.insert(sorted_pms.end(), pms.begin(), pms.end());
		}
		pms.clear();

		/// Sort virtual machines according to their shares
		vm_container sorted_vms(vms);
		::std::sort(sorted_vms.begin(),
					sorted_vms.end(),
					detail::ptr_virtual_machine_greater_comparator<vm_type>());

::std::cerr << "BEGIN Incremental Placement" << ::std::endl;//XXX
DCS_DEBUG_TRACE("BEGIN Incremental Placement");//XXX
DCS_DEBUG_TRACE("#Machines: " << sorted_pms.size());//XXX
DCS_DEBUG_TRACE("#VMs: " << sorted_vms.size());//XXX

		virtual_machines_placement<traits_type> deployment(dc.current_virtual_machines_placement());

		vm_iterator vm_end_it(sorted_vms.end());
		for (vm_iterator vm_it = sorted_vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			vm_pointer ptr_vm(*vm_it);

			// paranoid-check: valid pointer
			DCS_DEBUG_ASSERT( ptr_vm );

			application_type const& app(ptr_vm->guest_system().application());

			// Retrieve the share for every resource of the VM guest system
			share_container ref_shares(ptr_vm->guest_system().resource_shares());

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
				resource_share_map shares;
				for (share_iterator ref_share_it = ref_shares.begin(); ref_share_it != ref_share_end_it; ++ref_share_it)
				{
					physical_resource_category ref_category(ref_share_it->first);
					real_type ref_share(ref_share_it->second);
					if (this->reference_share_penalty() > 0)
					{
						ref_share -= ref_share*this->reference_share_penalty();
					}

					real_type ref_capacity(app.reference_resource(ref_category).capacity());
					real_type ref_threshold(app.reference_resource(ref_category).utilization_threshold());

					real_type actual_capacity(ptr_pm->resource(ref_category)->capacity());
					real_type actual_threshold(ptr_pm->resource(ref_category)->utilization_threshold());

					real_type share;
					share = ::dcs::eesim::scale_resource_share(ref_capacity,
															   ref_threshold,
															   actual_capacity,
															   actual_threshold,
															   ref_share);

					shares[ref_category] = share;
				}

				// Reference to actual resource utilization
				resource_utilization_map utils;
				{
					//FIXME: CPU resource category is hard-coded.
					physical_resource_category ref_category(cpu_resource_category);
					real_type ref_util(0);
					ref_util = app.performance_model().tier_measure(
									ptr_vm->guest_system().id(),
									utilization_performance_measure
						);

					real_type ref_capacity(app.reference_resource(ref_category).capacity());
					real_type actual_capacity(ptr_pm->resource(ref_category)->capacity());

					real_type util(0);
					util = scale_resource_share(ref_capacity,
												actual_capacity,
												ref_util,
												ptr_pm->resource(ref_category)->utilization_threshold());
					if (shares.count(ref_category))
					{
						util /= shares.at(ref_category);
					}

					utils[ref_category] = util;
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
::std::cerr << "Evaluating Placement of VM(" << ptr_vm->id() << ") over PM(" << ptr_pm->id() << ") ==> " <<  (placed ? "YES" : "NO") << ::std::endl;///XXX
			}
		}

DCS_DEBUG_TRACE("END Incremental Placement ==> " << deployment);///XXX
		return deployment;
	}
}; // best_fit_decreasing_incremental_placement_strategy

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BEST_FIT_DECREASING_INCREMENTAL_PLACEMENT_STRATEGY_HPP
