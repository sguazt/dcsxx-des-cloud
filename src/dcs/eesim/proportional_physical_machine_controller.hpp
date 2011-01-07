#ifndef DCS_EESIM_PROPORTIONAL_PHYSICAL_MACHINE_CONTROLLER_HPP
#define DCS_EESIM_PROPORTIONAL_PHYSICAL_MACHINE_CONTROLLER_HPP


#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/base_physical_machine_controller.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/macro.hpp>


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
	private: typedef registry<traits_type> registry_type;
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

	private: void do_process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		//TODO
	}

	//@} Interface Member Functions
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_PROPORTIONAL_PHYSICAL_MACHINE_CONTROLLER_HPP
