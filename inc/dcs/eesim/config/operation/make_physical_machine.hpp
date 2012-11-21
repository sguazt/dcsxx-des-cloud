#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_PHYSICAL_MACHINE_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_PHYSICAL_MACHINE_HPP


#include <dcs/eesim/config/operation/make_physical_resource.hpp>
#include <dcs/eesim/config/physical_machine.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim { namespace config {

template <typename TraitsT, typename RealT>
::dcs::shared_ptr<
	::dcs::eesim::physical_machine<TraitsT>
> make_physical_machine(physical_machine_config<RealT> const& machine_conf)
{
	typedef ::dcs::eesim::physical_machine<TraitsT> physical_machine_type;
	typedef physical_machine_config<RealT> physical_machine_config_type;

	::dcs::shared_ptr<physical_machine_type> ptr_mach;

	ptr_mach = ::dcs::make_shared<physical_machine_type>();
	if (!machine_conf.name.empty())
	{
		ptr_mach->name(machine_conf.name);
	}

	typedef typename physical_machine_config_type::resource_container::const_iterator iterator;
	iterator end_it = machine_conf.resources.end();
	for (iterator it = machine_conf.resources.begin(); it != end_it; ++it)
	{
		ptr_mach->add_resource(
			make_physical_resource<TraitsT>(it->second)
		);
	}

	return ptr_mach;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_PHYSICAL_MACHINE_HPP
