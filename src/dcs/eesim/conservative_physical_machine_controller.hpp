#ifndef DCS_EESIM_CONSERVATIVE_PHYSICAL_MACHINE_CONTROLLER_HPP
#define DCS_EESIM_CONSERVATIVE_PHYSICAL_MACHINE_CONTROLLER_HPP


#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/base_physical_machine_controller.hpp>
#include <dcs/eesim/registry.hpp>
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
	private: typedef registry<traits_type> registry_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;


	public: conservative_physical_machine_controller()
	: base_type()
	{
	}


	public: conservative_physical_machine_controller(physical_machine_pointer const& ptr_mach)
	: base_type(ptr_mach)
	{
	}


	//@{ Interface Member Functions

	private: void do_process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		//TODO
	}

	//@} Interface Member Functions
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_CONSERVATIVE_PHYSICAL_MACHINE_CONTROLLER_HPP
