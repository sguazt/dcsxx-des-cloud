#ifndef DCS_EESIM_CONSERVATIVE_PHYSICAL_MACHINE_CONTROLLER_HPP
#define DCS_EESIM_CONSERVATIVE_PHYSICAL_MACHINE_CONTROLLER_HPP


#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/base_physical_machine_controller.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/power_status.hpp>
//#include <dcs/eesim/registry.hpp>
#include <dcs/macro.hpp>


namespace dcs { namespace eesim {

/**
 * \brief Conservative physical machine controller.
 *
 * A physical machine controller which assigns new resources shares by
 * preserving already assigned resource shares.
 * For instance, if a virtual machine VM2 demands 80% of CPU, and supposing that
 * the CPU can be utilized at most for 90% and it is already been assigned for
 * 30% to another virtual machine VM1, the controller performs the following
 * allocations:
 * - VM1: 0.30 (untouched)
 * - VM2: min(0.80,(0.90-0.30)) = 0.60
 * .
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class conservative_physical_machine_controller: public base_physical_machine_controller<TraitsT>
{
	private: typedef base_physical_machine_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename base_type::physical_machine_pointer physical_machine_pointer;
	private: typedef typename traits_type::real_type real_type;
	private: typedef typename base_type::physical_machine_type physical_machine_type;
//	private: typedef registry<traits_type> registry_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename physical_machine_type::vmm_type vmm_type;
	private: typedef typename vmm_type::virtual_machine_type vm_type;
	private: typedef typename vm_type::identifier_type vm_identifier_type;
	private: typedef ::std::map<physical_resource_category,real_type> share_container;
	private: typedef ::std::map<vm_identifier_type,share_container> vm_share_container;


	public: conservative_physical_machine_controller()
	: base_type()
	{
	}


	public: conservative_physical_machine_controller(physical_machine_pointer const& ptr_mach)
	: base_type(ptr_mach)
	{
	}


	//@{ Interface Member Functions

	private: void do_control()
	{
#if 0
		// pre: physical machine must have already been set
		DCS_DEBUG_ASSERT( this->machine_ptr() );

		typedef typename vmm_type::virtual_machine_container vm_container;
		typedef typename vm_container::const_iterator vm_iterator;
		typedef typename share_container::const_iterator share_iterator;

		vm_container actual_vms(this->machine().vmm().virtual_machines(powered_on_power_status));

		vm_iterator vm_end_it(actual_vms.end());
		for (vm_iterator vm_it = actual_vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_pointer ptr_vm(*vm_it);
			vm_identifier_type vm_id(ptr_vm->id());
			share_container marginal_shares;

			if (vm_share_map_.count(vm_id) > 0)
			{
				// VM already present
				share_iterator share_end_it = vm_share_map_.at(vm_id).end();
				for (share_iterator share_it = vm_share_map_.at(vm_id).begin(); share_it != share_end_it; ++share_it)
				{
					physical_resource_category res_category(share_it->first);

					real_type old_wanted_share(share_it->second);
					real_type new_wanted_share(ptr_vm->wanted_resource_share(res_category));

					if (new_wanted_share > old_wanted_share)
					{
						// VM wants more share than before.
						// So, we need a two-step check since before assigning
						marginal_shares[category] = new_wanted_share-old_wanted_share;
					}
					else
					{
						// VM wants less share than before.
						// So, working conservatively, assign to this VM the
						// wanted VM if possibile or if not leave it unchanged.

						real_type actual_share(ptr_vm->resource_share(res_category));
						real_type new_share(::std::min(new_wanted_share, actual_share));

						ptr_vm->resource_share(res_category, new_share);
					}
				}
			}
		}

		//TODO: remove VMs no more powered-on
#endif
	}

	//@} Interface Member Functions


	private: vm_share_container vm_share_map_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_CONSERVATIVE_PHYSICAL_MACHINE_CONTROLLER_HPP
