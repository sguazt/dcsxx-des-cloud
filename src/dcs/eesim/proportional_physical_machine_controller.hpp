#ifndef DCS_EESIM_PROPORTIONAL_PHYSICAL_MACHINE_CONTROLLER_HPP
#define DCS_EESIM_PROPORTIONAL_PHYSICAL_MACHINE_CONTROLLER_HPP


#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/base_physical_machine_controller.hpp>
#include <dcs/memory.hpp>


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
	private: base_physical_machine_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	private: typedef physical_machine<traits_type> physical_machine_type;
	public: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;


	public: proportional_physical_machine_controller(physical_machine_pointer const& ptr_mach)
	: base_type(),
	  ptr_mach_(ptr_mach)
	{
	}


	private: physical_machine_pointer ptr_mach_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_PROPORTIONAL_PHYSICAL_MACHINE_CONTROLLER_HPP
