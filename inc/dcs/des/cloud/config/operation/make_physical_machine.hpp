#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PHYSICAL_MACHINE_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PHYSICAL_MACHINE_HPP


#include <dcs/des/cloud/config/operation/make_physical_resource.hpp>
#include <dcs/des/cloud/config/physical_machine.hpp>
#include <dcs/des/cloud/physical_machine.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace des { namespace cloud { namespace config {

template <typename TraitsT, typename RealT>
::dcs::shared_ptr<
	::dcs::des::cloud::physical_machine<TraitsT>
> make_physical_machine(physical_machine_config<RealT> const& machine_conf)
{
	typedef ::dcs::des::cloud::physical_machine<TraitsT> physical_machine_type;
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

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PHYSICAL_MACHINE_HPP
