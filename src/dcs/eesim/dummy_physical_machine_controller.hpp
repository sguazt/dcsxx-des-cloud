#ifndef DCS_EESIM_DUMMY_PHYSICAL_MACHINE_CONTROLLER_HPP
#define DCS_EESIM_DUMMY_PHYSICAL_MACHINE_CONTROLLER_HPP


#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/base_physical_machine_controller.hpp>
#include <dcs/macro.hpp>


namespace dcs { namespace eesim {

/**
 * \brief Dummy physical machine controller.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class dummy_physical_machine_controller: public base_physical_machine_controller<TraitsT>
{
	private: typedef base_physical_machine_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename base_type::physical_machine_pointer physical_machine_pointer;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;


	public: dummy_physical_machine_controller()
	: base_type()
	{
	}


	public: dummy_physical_machine_controller(physical_machine_pointer const& ptr_mach)
	: base_type(ptr_mach)
	{
	}


    private: void do_process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		// empty
	}
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_DUMMY_PHYSICAL_MACHINE_CONTROLLER_HPP
