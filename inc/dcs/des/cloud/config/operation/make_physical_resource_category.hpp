#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PHYSICAL_RESOURCE_CATEGORY_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PHYSICAL_RESOURCE_CATEGORY_HPP


#include <dcs/des/cloud/config/physical_resource.hpp>
#include <dcs/des/cloud/physical_resource_category.hpp>
////#include <dcs/exception.hpp>
#include <stdexcept>


namespace dcs { namespace des { namespace cloud { namespace config {

::dcs::des::cloud::physical_resource_category make_physical_resource_category(physical_resource_category category)
{
	switch (category)
	{
		case cpu_resource:
			return ::dcs::des::cloud::cpu_resource_category;
			break;
		case mem_resource:
			return ::dcs::des::cloud::memory_resource_category;
			break;
		case disk_resource:
			return ::dcs::des::cloud::storage_resource_category;
			break;
		case nic_resource:
//			return ::dcs::des::cloud::network_resource_category;
//			return ::dcs::des::cloud::network_up_resource_category;
//			return ::dcs::des::cloud::network_down_resource_category;
			//DCS_EXCEPTION_THROW(::std::runtime_error, "NIC resource not yet handled.");
			throw ::std::runtime_error("NIC resource not yet handled.");
			break;
	}

	//DCS_EXCEPTION_THROW(::std::runtime_error, "Unknown physical resource category.");
	throw ::std::runtime_error("Unknown physical resource category.");
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PHYSICAL_RESOURCE_CATEGORY_HPP
