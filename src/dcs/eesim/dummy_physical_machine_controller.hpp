#ifndef DCS_EESIM_DUMMY_PHYSICAL_MACHINE_CONTROLLER_HPP
#define DCS_EESIM_DUMMY_PHYSICAL_MACHINE_CONTROLLER_HPP


#include <dcs/eesim/base_physical_machine_controller.hpp>


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


	public: dummy_physical_machine_controller()
	: base_type()
	{
	}


	public: dummy_physical_machine_controller(physical_machine_pointer const& ptr_mach)
	: base_type(ptr_mach)
	{
	}
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_DUMMY_PHYSICAL_MACHINE_CONTROLLER_HPP
