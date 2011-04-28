#ifndef DCS_EESIM_FIRST_FIT_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_EESIM_FIRST_FIT_INITIAL_PLACEMENT_STRATEGY_HPP


#include <cstddef>
#include <dcs/eesim/base_initial_placement_strategy.hpp>
#include <dcs/eesim/data_center.hpp>
//#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
//#include <dcs/eesim/physical_resource_view.hpp>
#include <dcs/eesim/utility.hpp>
#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>
#include <dcs/memory.hpp>
#include <utility>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class first_fit_initial_placement_strategy: public base_initial_placement_strategy<TraitsT>
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;


	private: virtual_machines_placement<traits_type> do_placement(data_center<traits_type> const& dc)
	{
		typedef physical_machine<traits_type> physical_machine_type;
		typedef virtual_machine<traits_type> virtual_machine_type;
		typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
		typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
		typedef typename physical_machine_type::identifier_type pm_identifier_type;
		typedef typename virtual_machine_type::identifier_type vm_identifier_type;
		typedef ::std::vector<physical_machine_pointer> pm_container;
		typedef ::std::vector<virtual_machine_pointer> vm_container;
		typedef typename pm_container::const_iterator pm_iterator;
		typedef typename vm_container::const_iterator vm_iterator;
//		typedef typename virtual_machine_type::resource_share_container share_container;
		typedef ::std::pair<physical_resource_category,real_type> share_type;
		typedef ::std::vector<share_type> share_container;
		typedef typename share_container::const_iterator share_iterator;
		typedef multi_tier_application<traits_type> application_type; 
//		typedef physical_resource_view<traits_type> physical_resource_view_type;
//		typedef ::std::vector<physical_resource_view_type> resource_view_container;

		pm_container machs =  dc.physical_machines();
		vm_container vms =  dc.virtual_machines();

DCS_DEBUG_TRACE("BEGIN Initial Placement");//XXX
DCS_DEBUG_TRACE("#Machines: " << machs.size());//XXX
DCS_DEBUG_TRACE("#VMs: " << vms.size());//XXX

		virtual_machines_placement<traits_type> deployment;

		vm_iterator vm_end_it = vms.end();
		for (vm_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_pointer ptr_vm = *vm_it;
			//pm_identifier_type mach_id;
			physical_machine_pointer ptr_mach;
			application_type const& app(ptr_vm->guest_system().application());
			bool placed = false;

			// Retrieve the share for every resource of the VM guest system
			share_container ref_shares(ptr_vm->guest_system().resource_shares());
//			resource_view_container ref_resources(ptr_vm->guest_system().application().reference_resources());

			// For each physical machine PM, try to deploy current VM on PM
			// until a suitable machine (i.e., a machine with sufficient free
			// capacity) is found.
			pm_iterator pm_end_it = machs.end();
			share_iterator ref_share_end_it = ref_shares.end();
			for (pm_iterator pm_it = machs.begin(); pm_it != pm_end_it && !placed; ++pm_it)
			{
//				mach_id = (*pm_it)->id();
				ptr_mach = *pm_it;

				// Reference to actual resource shares
				share_container shares;
				for (share_iterator ref_share_it = ref_shares.begin(); ref_share_it != ref_share_end_it; ++ref_share_it)
				{
					physical_resource_category ref_category(ref_share_it->first);
					real_type ref_share(ref_share_it->second);
					ref_share -= ref_share*this->reference_share_penalty();

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

					shares.push_back(share_type(ref_category, share));
				}

				// Try to place current VM on current PM
				placed = deployment.try_place(*ptr_vm,
											  *ptr_mach,
											  shares.begin(),
											  shares.end());

			}

//DCS_DEBUG_TRACE("Before placing: " << vm_id << " -> " << ptr_mach->id() << " ==> OK? " << std::boolalpha << placeable);///XXX
//			if (placeable)
//			{
//				deployment.place(*ptr_vm,
//								 *ptr_mach,
//								 shares.begin(),
//								 shares.end());
//			}
DCS_DEBUG_TRACE("Placed: VM(" << ptr_vm->id() << ") -> PM(" << ptr_mach->id() << ") ==> OK? " <<  std::boolalpha << placed);///XXX
		}

DCS_DEBUG_TRACE("END Initial Placement ==> " << deployment);///XXX
		return deployment;
	}
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_FIRST_FIT_INITIAL_PLACEMENT_STRATEGY_HPP
