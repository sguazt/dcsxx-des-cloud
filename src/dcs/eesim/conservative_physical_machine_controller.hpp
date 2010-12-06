#ifndef DCS_EESIM_CONSERVATIVE_PHYSICAL_MACHINE_CONTROLLER_HPP
#define DCS_EESIM_CONSERVATIVE_PHYSICAL_MACHINE_CONTROLLER_HPP


#include <dcs/eesim/base_physical_machine_controller.hpp>


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


	public: conservative_physical_machine_controller()
	: base_type()
	{
	}


	public: conservative_physical_machine_controller(physical_machine_pointer const& ptr_mach)
	: base_type(ptr_mach)
	{
	}
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_CONSERVATIVE_PHYSICAL_MACHINE_CONTROLLER_HPP
