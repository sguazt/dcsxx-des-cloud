#ifndef DCS_EESIM_BEST_FIT_INCREMENTAL_PLACEMENT_STRATEGY_HPP
#define DCS_EESIM_BEST_FIT_INCREMENTAL_PLACEMENT_STRATEGY_HPP


#include <dcs/debug.hpp>
#include <dcs/eesim/base_incremental_placement_strategy.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/detail/placement_strategy_utility.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/utility.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>
#include <utility>
#include <vector>


//FIXME:
// - Fit is based only on the CPU resource (see detail::pm_comparator)
//


namespace dcs { namespace eesim {

/// This is substantially a First-Fit decreasing heuristic.
template <typename TraitsT>
class best_fit_incremental_placement_strategy: public base_incremental_placement_strategy<TraitsT>
{
	private: typedef base_incremental_placement_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	private: typedef typename base_type::virtual_machine_identifier_type vm_identifier_type;
	private: typedef typename base_type::virtual_machine_identifier_container vm_identifier_container;
	private: typedef data_center<traits_type> data_center_type;


	private: virtual_machines_placement<traits_type> do_place(data_center_type const& dc, vm_identifier_container const& vms)
	{
		typedef typename data_center_type::application_type application_type;
		typedef typename data_center_type::physical_machine_type physical_machine_type;
		typedef typename data_center_type::physical_machine_pointer physical_machine_pointer;
		typedef typename physical_machine_type::identifier_type pm_identifier_type;
		typedef ::std::vector<physical_machine_pointer> pm_container;
		typedef typename pm_container::const_iterator pm_iterator;
		typedef typename vm_identifier_container::const_iterator vm_identifier_iterator;
		typedef ::std::pair<physical_resource_category,real_type> share_type;
		typedef ::std::vector<share_type> share_container;
		typedef typename share_container::const_iterator share_iterator;

		pm_container machs(dc.physical_machines());

DCS_DEBUG_TRACE("BEGIN Incremental Placement");//XXX
DCS_DEBUG_TRACE("#Machines: " << machs.size());//XXX
DCS_DEBUG_TRACE("#VMs: " << vms.size());//XXX

		virtual_machines_placement<traits_type> deployment;

		// Sort physical machines according to their capacity
		// (from the less powerful to the more powerful)
		::std::sort(machs.begin(), machs.end(), detail::pm_comparator<physical_machine_type>());

		vm_identifier_iterator vm_end_it(vms.end());
		for (vm_identifier_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			vm_identifier_type vm_id(*vm_it);

			virtual_machine_pointer ptr_vm(dc.virtual_machine(vm_id));

			// paranoid-check: valid pointer
			DCS_DEBUG_ASSERT( ptr_vm );

			application_type const& app(ptr_vm->guest_system().application());

			// Retrieve the share for every resource of the VM guest system
			share_container ref_shares(ptr_vm->guest_system().resource_shares());

			// For each physical machine PM, try to deploy current VM on PM
			// until a suitable machine (i.e., a machine with sufficient free
			// capacity) is found.
			bool placed(false);
			pm_iterator pm_end_it(machs.end());
			share_iterator ref_share_end_it(ref_shares.end());
			for (pm_iterator pm_it = machs.begin(); pm_it != pm_end_it && !placed; ++pm_it)
			{
				physical_machine_pointer ptr_mach(*pm_it);

				// paranoid-check: valid pointer
				DCS_DEBUG_ASSERT( ptr_mach );

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
					real_type ref_threshold(app.reference_resource(ref_category).utilization_threshold());

					real_type actual_capacity(ptr_mach->resource(ref_category)->capacity());
					real_type actual_threshold(ptr_mach->resource(ref_category)->utilization_threshold());

					real_type share;
					share = ::dcs::eesim::scale_resource_share(ref_capacity,
															   ref_threshold,
															   actual_capacity,
															   actual_threshold,
															   ref_share);

					shares.push_back(::std::make_pair(ref_category, share));
				}

				// Try to place current VM on current PM
				placed = deployment.try_place(*ptr_vm,
											  *ptr_mach,
											  shares.begin(),
											  shares.end());
DCS_DEBUG_TRACE("Placed: VM(" << ptr_vm->id() << ") -> PM(" << ptr_mach->id() << ") ==> OK? " <<  std::boolalpha << placed);///XXX
			}
		}

DCS_DEBUG_TRACE("END Incremental Placement ==> " << deployment);///XXX
		return deployment;
	}
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BEST_FIT_INCREMENTAL_PLACEMENT_STRATEGY_HPP
