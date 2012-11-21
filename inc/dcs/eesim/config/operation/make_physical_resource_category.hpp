#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_PHYSICAL_RESOURCE_CATEGORY_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_PHYSICAL_RESOURCE_CATEGORY_HPP


#include <dcs/eesim/config/physical_resource.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
////#include <dcs/exception.hpp>
#include <stdexcept>


namespace dcs { namespace eesim { namespace config {

::dcs::eesim::physical_resource_category make_physical_resource_category(physical_resource_category category)
{
	switch (category)
	{
		case cpu_resource:
			return ::dcs::eesim::cpu_resource_category;
			break;
		case mem_resource:
			return ::dcs::eesim::memory_resource_category;
			break;
		case disk_resource:
			return ::dcs::eesim::storage_resource_category;
			break;
		case nic_resource:
//			return ::dcs::eesim::network_resource_category;
//			return ::dcs::eesim::network_up_resource_category;
//			return ::dcs::eesim::network_down_resource_category;
			//DCS_EXCEPTION_THROW(::std::runtime_error, "NIC resource not yet handled.");
			throw ::std::runtime_error("NIC resource not yet handled.");
			break;
	}

	//DCS_EXCEPTION_THROW(::std::runtime_error, "Unknown physical resource category.");
	throw ::std::runtime_error("Unknown physical resource category.");
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_PHYSICAL_RESOURCE_CATEGORY_HPP
