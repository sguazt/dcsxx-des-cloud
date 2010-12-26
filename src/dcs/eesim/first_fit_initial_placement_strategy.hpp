#ifndef DCS_EESIM_FIRST_FIT_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_EESIM_FIRST_FIT_INITIAL_PLACEMENT_STRATEGY_HPP


#include <cstddef>
#include <dcs/eesim/base_initial_placement_strategy.hpp>
#include <dcs/eesim/data_center.hpp>
//#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>
#include <dcs/memory.hpp>
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
		typedef typename virtual_machine_type::resource_share_container share_container;

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
			bool placed = false;

//			share_container shares;
			pm_iterator pm_end_it = machs.end();
			for (pm_iterator pm_it = machs.begin(); pm_it != pm_end_it && !placed; ++pm_it)
			{
//				mach_id = (*pm_it)->id();
				ptr_mach = *pm_it;

				share_container shares = ptr_vm->wanted_resource_shares(*ptr_mach);

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
