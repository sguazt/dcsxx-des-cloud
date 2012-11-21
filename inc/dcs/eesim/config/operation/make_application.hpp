#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_APPLICATION_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_APPLICATION_HPP


#include <boost/variant.hpp>
#include <cstddef>
#include <dcs/eesim/application_tier.hpp>
#include <dcs/eesim/config/application.hpp>
#include <dcs/eesim/config/configuration.hpp>
#include <dcs/eesim/config/operation/make_application_performance_model.hpp>
#include <dcs/eesim/config/operation/make_application_simulation_model.hpp>
#include <dcs/eesim/config/operation/make_application_sla_cost_model.hpp>
#include <dcs/eesim/config/operation/make_physical_resource_category.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim { namespace config {

namespace detail { namespace /*<unnamed>*/ {

//FIXME: utilization threshold not yet handled.
template <typename TraitsT, typename RealT, typename UIntT>
::dcs::shared_ptr<
	::dcs::eesim::multi_tier_application<TraitsT>
> make_application(::dcs::eesim::config::application_config<RealT,UIntT> const& app_conf,
				   ::dcs::eesim::config::configuration<RealT,UIntT> const& conf,
				   ::dcs::shared_ptr<typename TraitsT::uniform_random_generator_type> const& ptr_rng,
				   ::dcs::shared_ptr<typename TraitsT::des_engine_type> const& ptr_engine)
{
	typedef TraitsT traits_type;
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef ::dcs::eesim::multi_tier_application<traits_type> application_type;
	typedef ::dcs::eesim::config::application_config<real_type,uint_type> application_config_type;

	::dcs::shared_ptr<application_type> ptr_app;

	ptr_app = ::dcs::make_shared<application_type>();

	// Name
	if (!app_conf.name.empty())
	{
		ptr_app->name(app_conf.name);
	}

	// Tiers
	{
		typedef ::dcs::eesim::application_tier<traits_type> tier_type;
		typedef typename application_config_type::tier_config_type tier_config_type;
		typedef typename application_config_type::tier_container::const_iterator iterator;
		typedef typename tier_config_type::share_container::const_iterator share_iterator;

		iterator end_it = app_conf.tiers.end();
		for (iterator it = app_conf.tiers.begin(); it != end_it; ++it)
		{
			::dcs::shared_ptr<tier_type> ptr_tier;

			ptr_tier = ::dcs::make_shared<tier_type>();

			ptr_tier->name(it->name);

			share_iterator share_end_it = it->shares.end();
			for (share_iterator share_it = it->shares.begin(); share_it != share_end_it; ++share_it)
			{
				ptr_tier->resource_share(
					::dcs::eesim::config::make_physical_resource_category(share_it->first),
					share_it->second
				);
			}

			ptr_app->tier(ptr_tier);
		}
	}

	// Reference resources
	{
		typedef typename application_config_type::reference_resource_container::const_iterator iterator;
		iterator end_it = app_conf.reference_resources.end();
		for (iterator it = app_conf.reference_resources.begin(); it != end_it; ++it)
		{
			ptr_app->reference_resource(
				::dcs::eesim::config::make_physical_resource_category(it->first),
				it->second,
				RealT(1)
			);
		}
	}

	// SLA
	ptr_app->sla_cost_model(
		make_application_sla_cost_model<TraitsT>(app_conf.sla)
	);

	// Performance model
	ptr_app->performance_model(
		make_application_performance_model<TraitsT>(app_conf.perf_model)
	);

	// Simulation model
	ptr_app->simulation_model(
		make_application_simulation_model<TraitsT>(
			app_conf,
			*ptr_app,
			conf,
			ptr_rng,
			ptr_engine
		)
	);

	return ptr_app;
}

}} // Namespace detail::<unnamed>


template <typename TraitsT, typename RealT, typename UIntT>
inline
::dcs::shared_ptr<
	::dcs::eesim::multi_tier_application<TraitsT>
> make_application(application_config<RealT,UIntT> const& app_conf,
				   configuration<RealT,UIntT> const& conf,
				   ::dcs::shared_ptr<typename TraitsT::uniform_random_generator_type> const& ptr_rng,
				   ::dcs::shared_ptr<typename TraitsT::des_engine_type> const& ptr_des_eng)
{
	return detail::make_application<TraitsT>(app_conf, conf, ptr_rng, ptr_des_eng);
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_APPLICATION_HPP
