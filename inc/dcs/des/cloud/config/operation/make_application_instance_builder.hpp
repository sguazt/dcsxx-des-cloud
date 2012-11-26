#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_APPLICATION_INSTANCE_BUILDER_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_APPLICATION_INSTANCE_BUILDER_HPP


#include <dcs/eesim/base_application_instance_builder.hpp>
#include <dcs/eesim/configuration_based_application_instance_builder.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim { namespace config {

template <typename TraitsT, typename RealT, typename UIntT>
::dcs::shared_ptr<
	::dcs::eesim::base_application_instance_builder<TraitsT>
> make_application_instance_builder(application_config<RealT,UIntT> const& app_conf)
{
	typedef ::dcs::eesim::configuration_based_application_instance_builder<TraitsT> impl_type;

	return ::dcs::make_shared<impl_type>(app_conf);
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_APPLICATION_INSTANCE_BUILDER_HPP
