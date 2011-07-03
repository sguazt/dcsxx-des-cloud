#ifndef DCS_EESIM_PROPORTIONAL_PHYSICAL_MACHINE_CONTROLLER_HPP
#define DCS_EESIM_PROPORTIONAL_PHYSICAL_MACHINE_CONTROLLER_HPP


#include <algorithm>
#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/base_physical_machine_controller.hpp>
#include <dcs/macro.hpp>
#include <map>
//[XXX]
#ifdef DCS_EESIM_EXP_OUTPUT_VM_SHARES
#include <cstdlib>
#include <iosfwd>
#include <fstream>
#include <sstream>
#include <string>
#endif // DCS_EESIM_EXP_OUTPUT_VM_SHARES
//[/XXX]


namespace dcs { namespace eesim {

/**
 * \brief Proportional physical machine controller.
 *
 * A physical machine controller which assigns resources shares according to
 * the incoming resources demands.
 * For instance, if two virtual machines demand 30% and 80% of CPU,
 * respectively, and supposing that the CPU can be utilized at most for 90%,
 * the controller performs the following allocations:
 * - VM1: (0.3/(0.3+0.8))*0.9 ~= 0.24545...
 * - VM2: (0.8/(0.3+0.8))*0.9 ~= 0.65454...
 * .
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class proportional_physical_machine_controller: public base_physical_machine_controller<TraitsT>
{
	private: typedef base_physical_machine_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename base_type::physical_machine_pointer physical_machine_pointer;
	private: typedef typename traits_type::real_type real_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


	public: proportional_physical_machine_controller()
	: base_type()
	{
	}


	public: proportional_physical_machine_controller(physical_machine_pointer const& ptr_mach)
	: base_type(ptr_mach)
	{
	}


	//@{ Interface Member Functions

	private: void do_control()
	{
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

//		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Processing CONTROL (Clock: " << ctx.simulated_time() << ")");//XXX

        // pre: physical machine must have already been set
        DCS_DEBUG_ASSERT( this->machine_ptr() );

		typedef typename base_type::physical_machine_type physical_machine_type;
		typedef typename physical_machine_type::vmm_type vmm_type;
		typedef typename vmm_type::virtual_machine_type vm_type;
		typedef typename vmm_type::virtual_machine_pointer vm_pointer;
		typedef typename vmm_type::virtual_machine_container vm_container;
		typedef typename vm_container::const_iterator vm_iterator;
		typedef typename vm_type::resource_share_container share_container;
		typedef typename share_container::const_iterator share_iterator;
		typedef ::std::map<physical_resource_category,real_type> share_map_container;

		vm_container actual_vms(this->machine().vmm().virtual_machines(powered_on_power_status));
		share_map_container share_sums;
		short step = 1;

		do
		{
			// Share assignment is done in 2 steps
			// 1. In the first step computes the total resource fraction
			//    requested by all VMs. The computed value both serves to check
			//    if the total fraction overpass the maximum allowable
			//    utilization and as a normalization constant.
			// 2. In the second step assign resource shares.
			//    Specifically, if the total amount of requested resource
			//    capacity is greater than the one assignable then
			vm_iterator vm_end_it(actual_vms.end());
			for (vm_iterator vm_it = actual_vms.begin(); vm_it != vm_end_it; ++vm_it)
			{
				vm_pointer ptr_vm(*vm_it);
				share_container vm_wanted_shares(ptr_vm->wanted_resource_shares());
				share_iterator share_end_it(vm_wanted_shares.end());
				for (share_iterator share_it = vm_wanted_shares.begin(); share_it != share_end_it; ++share_it)
				{
					physical_resource_category category(share_it->first);
					real_type share(share_it->second);

					if (step == 1)
					{
						// Compute share sum for this resource category

						if (share_sums.count(category) > 0)
						{
							share_sums[category] += share;
						}
						else
						{
							share_sums[category] = share;
						}
					}
					else
					{
						// Assign share for this resource category

						real_type share_sum(share_sums.at(category));
						real_type threshold(this->machine().resource(category)->utilization_threshold());

						if (share_sum > threshold)
						{
							// Assign share proportionally
							share *= threshold/share_sum;
							// The operation below may appear useless. However it is needed to compensate spurious decimal digit derived from the above product.
							share = ::std::min(share, threshold);
						}
						// ... else Assign share as it is originally requested.

						// check: make sure resource share is bounded in [0,1]
						//        and respects the utilization threshold.
						DCS_DEBUG_ASSERT( share >= real_type(0) && share <= threshold && share <= real_type(1) );

//::std::cerr << "APP: " << ptr_vm->guest_system().application().id() << ", MACH: " << this->machine().id() << ", VM: " << ptr_vm->id() << ", WantedShare: " << share_it->second << ", GotShare: " << share << ::std::endl;//XXX
						ptr_vm->resource_share(category, share);

						DCS_DEBUG_TRACE("APP: " << ptr_vm->guest_system().application().id() << ", MACH: " << this->machine().id() << " - Assigned new share: VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Category: " << category << " - Threshold: " << threshold << " - Share Sum: " << share_sum << " ==> Wanted: " << share_it->second << " - Got: " << share);//XXX
//::std::cerr << "APP: " << ptr_vm->guest_system().application().id() << ", MACH: " << this->machine().id() << " - Assigned new share: VM: " << ptr_vm->name() << " (" << ptr_vm->id() << ") - Category: " << category << " - Threshold: " << threshold << " - Share Sum: " << share_sum << " ==> Wanted: " << share_it->second << " - Got: " << share << ::std::endl;//XXX
//::std::cerr << "APP: " << ptr_vm->guest_system().application().id() << ", MACH: " << this->machine().id() << ", VM: " << ptr_vm->id() << ", WantedShare: " << share_it->second << ", GotShare: " << share << ::std::endl;//XXX

//[XXX]
#ifdef DCS_EESIM_EXP_OUTPUT_VM_SHARES
						typedef typename traits_type::uint_type uint_type;
						uint_type nrep = dynamic_cast< ::dcs::des::replications::engine<real_type,uint_type>* >(registry<traits_type>::instance().des_engine_ptr().get())->num_replications();
						if (nrep == 1)
						{
							::std::cerr << ::std::endl << ">>>>>> IMPORTANT <<<<<<<" << ::std::endl;
							::std::cerr << ::std::endl << "REMOVE THE CODE WHICH OUTPUT VM SHARES TO A FILE!!" << ::std::endl;
							::std::cerr << ::std::endl << "(or put it between an #ifdef-#endif block)" << ::std::endl;
							::std::cerr << ::std::endl << "-------------------------" << ::std::endl;
							::std::ostringstream oss;
							oss << "vmshares-" << ptr_vm->id() << "-" << ::std::string(::getenv("CONDOR_JOB_ID")) << ".dat";
							::std::ofstream ofs(oss.str().c_str(), ::std::ios_base::app);
							ofs << ptr_vm->id() << "," << category << "," << share_it->second << "," << share << ::std::endl;
							// sim-time, vm-id, mach-id, resource-category, wanted-share, assigned-share
							 ofs << registry<traits_type>::instance().des_engine_ptr()->simulated_time() << "," << ptr_vm->id() << "," << this->machine().id() << "," << category << "," << share_it->second << "," << share << ::std::endl;
							ofs.close();
						}
#endif // DCS_EESIM_EXP_OUTPUT_VM_SHARES
//[/XXX]
					}
				}
			}

			++step;
		}
		while (step <= 2);
//
//		DCS_DEBUG_TRACE("(" << this << ") END Do Processing CONTROL (Clock: " << ctx.simulated_time() << ")");//XXX
	}


//	private: void do_process_control(des_event_type const& evt, des_engine_context_type& ctx)
//	{
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
//		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );
//
//		DCS_DEBUG_TRACE("(" << this << ") BEGIN Do Processing CONTROL (Clock: " << ctx.simulated_time() << ")");//XXX
//
//		this->control();
//
//		DCS_DEBUG_TRACE("(" << this << ") END Do Processing CONTROL (Clock: " << ctx.simulated_time() << ")");//XXX
//	}

	//@} Interface Member Functions
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_PROPORTIONAL_PHYSICAL_MACHINE_CONTROLLER_HPP
