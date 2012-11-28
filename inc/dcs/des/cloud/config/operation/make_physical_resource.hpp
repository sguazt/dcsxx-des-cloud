#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PHYSICAL_RESOURCE_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PHYSICAL_RESOURCE_HPP


#include <boost/variant.hpp>
#include <dcs/des/cloud/config/operation/make_energy_model.hpp>
#include <dcs/des/cloud/config/operation/make_physical_resource_category.hpp>
#include <dcs/des/cloud/config/physical_resource.hpp>
#include <dcs/des/cloud/physical_resource.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace des { namespace cloud { namespace config {

template <typename TraitsT, typename RealT>
::dcs::shared_ptr<
	::dcs::des::cloud::physical_resource<TraitsT>
> make_physical_resource(physical_resource_config<RealT> const& resource_conf)
{
	typedef ::dcs::des::cloud::physical_resource<TraitsT> physical_resource_type;
	typedef physical_resource_config<RealT> physical_resource_config_type;

	::dcs::shared_ptr<physical_resource_type> ptr_res;

	ptr_res = ::dcs::make_shared<physical_resource_type>();
	if (!resource_conf.name.empty())
	{
		ptr_res->name(resource_conf.name);
	}
	ptr_res->category(
		make_physical_resource_category(resource_conf.type)
	);
	ptr_res->capacity(resource_conf.capacity);
	ptr_res->utilization_threshold(resource_conf.threshold);
	ptr_res->energy_model(
		make_energy_model<RealT>(
			resource_conf.energy_model_type,
			resource_conf.energy_model_conf
		)
	);

	return ptr_res;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PHYSICAL_RESOURCE_HPP
