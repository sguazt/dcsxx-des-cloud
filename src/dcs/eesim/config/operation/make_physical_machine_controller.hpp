#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_PHYSICAL_MACHINE_CONTROLLER_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_PHYSICAL_MACHINE_CONTROLLER_HPP


#include <dcs/eesim/base_physical_machine_controller.hpp>
#include <dcs/eesim/config/physical_machine_controller.hpp>
#include <dcs/eesim/conservative_physical_machine_controller.hpp>
#include <dcs/eesim/dummy_physical_machine_controller.hpp>
#include <dcs/eesim/proportional_physical_machine_controller.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim { namespace config {

template <typename TraitsT, typename RealT>
::dcs::shared_ptr<
	::dcs::eesim::base_physical_machine_controller<TraitsT>
> make_physical_machine_controller(physical_machine_controller_config<RealT> const& controller_conf)
{
	typedef TraitsT traits_type;
	typedef ::dcs::eesim::base_physical_machine_controller<traits_type> controller_type;
	typedef physical_machine_controller_config<RealT> controller_config_type;

	::dcs::shared_ptr<controller_type> ptr_controller;

	switch (controller_conf.category)
	{
		case conservative_physical_machine_controller:
			{
				//typedef typename controller_config_type::conservative_controller_config_type controller_config_impl_type;
				typedef ::dcs::eesim::conservative_physical_machine_controller<traits_type> controller_impl_type;

				//controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				// Note: there is nothing to configure

				ptr_controller = ::dcs::make_shared<controller_impl_type>();
			}
			break;
		case proportional_physical_machine_controller:
			{
				//typedef typename controller_config_type::proportional_controller_config_type controller_config_impl_type;
				typedef ::dcs::eesim::proportional_physical_machine_controller<traits_type> controller_impl_type;

				//controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				// Note: there is nothing to configure

				ptr_controller = ::dcs::make_shared<controller_impl_type>();
			}
			break;
		case dummy_physical_machine_controller:
			{
				//typedef typename controller_config_type::dummy_controller_config_type controller_config_impl_type;
				typedef ::dcs::eesim::dummy_physical_machine_controller<traits_type> controller_impl_type;

				//controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				// Note: there is nothing to configure

				ptr_controller = ::dcs::make_shared<controller_impl_type>();
			}
			break;
	}

	ptr_controller->sampling_time(controller_conf.sampling_time);

	return ptr_controller;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_PHYSICAL_MACHINE_CONTROLLER_HPP
