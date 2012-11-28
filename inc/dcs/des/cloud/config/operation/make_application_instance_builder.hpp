#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_APPLICATION_INSTANCE_BUILDER_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_APPLICATION_INSTANCE_BUILDER_HPP


#include <dcs/des/cloud/base_application_instance_builder.hpp>
#include <dcs/des/cloud/configuration_based_application_instance_builder.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace des { namespace cloud { namespace config {

template <typename TraitsT, typename RealT, typename UIntT>
::dcs::shared_ptr<
	::dcs::des::cloud::base_application_instance_builder<TraitsT>
> make_application_instance_builder(application_config<RealT,UIntT> const& app_conf)
{
	typedef ::dcs::des::cloud::configuration_based_application_instance_builder<TraitsT> impl_type;

	return ::dcs::make_shared<impl_type>(app_conf);
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_APPLICATION_INSTANCE_BUILDER_HPP
