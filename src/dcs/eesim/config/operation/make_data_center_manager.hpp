#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_DATA_CENTER_MANAGER_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_DATA_CENTER_MANAGER_HPP


#include <boost/variant.hpp>
#include <dcs/eesim/base_initial_placement_strategy.hpp>
#include <dcs/eesim/base_migration_controller.hpp>
#include <dcs/eesim/best_fit_initial_placement_strategy.hpp>
#include <dcs/eesim/config/configuration.hpp>
#include <dcs/eesim/config/initial_placement_strategy.hpp>
#include <dcs/eesim/config/migration_controller.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/data_center_manager.hpp>
#include <dcs/eesim/first_fit_initial_placement_strategy.hpp>
#include <dcs/eesim/minlp_migration_controller.hpp>
#include <dcs/eesim/first_fit_scaleout_initial_placement_strategy.hpp>
#include <dcs/memory.hpp>
#include <stdexcept>


namespace dcs { namespace eesim { namespace config {

namespace detail { namespace /*<unnamed>*/ {

template <typename TraitsT, typename RealT>
::dcs::shared_ptr< ::dcs::eesim::base_initial_placement_strategy<TraitsT> > make_initial_placement_strategy(initial_placement_strategy_config<RealT> const& strategy_conf)
{
	typedef TraitsT traits_type;
	typedef ::dcs::eesim::base_initial_placement_strategy<traits_type> strategy_type;
	typedef initial_placement_strategy_config<RealT> strategy_config_type;

	::dcs::shared_ptr<strategy_type> ptr_strategy;

	switch (strategy_conf.category)
	{
		case best_fit_initial_placement_strategy:
			{
				//typedef typename strategy_config_type::best_fit_initial_placement_strategy_config_type strategy_config_impl_type;
				typedef ::dcs::eesim::best_fit_initial_placement_strategy<traits_type> strategy_impl_type;

				//strategy_config_impl_type const& strategy_conf_impl = ::boost::get<strategy_config_impl_type>(strategy_conf.category_conf);

				// Note: there is nothing to configure

				ptr_strategy = ::dcs::make_shared<strategy_impl_type>();
			}
			break;
		case first_fit_initial_placement_strategy:
			{
				//typedef typename strategy_config_type::first_fit_initial_placement_strategy_config_type strategy_config_impl_type;
				typedef ::dcs::eesim::first_fit_initial_placement_strategy<traits_type> strategy_impl_type;

				//strategy_config_impl_type const& strategy_conf_impl = ::boost::get<strategy_config_impl_type>(strategy_conf.category_conf);

				// Note: there is nothing to configure

				ptr_strategy = ::dcs::make_shared<strategy_impl_type>();
			}
			break;
		case first_fit_scaleout_initial_placement_strategy:
			{
				//typedef typename strategy_config_type::first_fit_scaleout_initial_placement_strategy_config_type strategy_config_impl_type;
				typedef ::dcs::eesim::first_fit_scaleout_initial_placement_strategy<traits_type> strategy_impl_type;

				//strategy_config_impl_type const& strategy_conf_impl = ::boost::get<strategy_config_impl_type>(strategy_conf.category_conf);

				// Note: there is nothing to configure

				ptr_strategy = ::dcs::make_shared<strategy_impl_type>();
			}
			break;
	}

	ptr_strategy->reference_share_penalty(strategy_conf.ref_penalty);

	return ptr_strategy;
}


template <typename TraitsT, typename RealT>
::dcs::shared_ptr< ::dcs::eesim::base_migration_controller<TraitsT> > make_migration_controller(migration_controller_config<RealT> const& controller_conf, ::dcs::shared_ptr< ::dcs::eesim::data_center<TraitsT> > const& ptr_dc)
{
	typedef TraitsT traits_type;
	typedef ::dcs::eesim::base_migration_controller<traits_type> controller_type;
	typedef migration_controller_config<RealT> controller_config_type;
	typedef ::dcs::eesim::data_center<TraitsT> data_center_type;
	typedef ::dcs::shared_ptr<data_center_type> data_center_pointer;

	::dcs::shared_ptr<controller_type> ptr_controller;

	switch (controller_conf.category)
	{
		case minlp_migration_controller:
			{
				//typedef typename controller_config_type::minlp_migration_controller_config_type controller_config_impl_type;
				typedef ::dcs::eesim::minlp_migration_controller<traits_type> controller_impl_type;

				//controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				// Note: there is nothing to configure

				ptr_controller = ::dcs::make_shared<controller_impl_type>(ptr_dc, controller_conf.sampling_time);
			}
			break;
		case dummy_migration_controller:
			{
				//typedef typename controller_config_type::minlp_migration_controller_config_type controller_config_impl_type;
				typedef ::dcs::eesim::dummy_migration_controller<traits_type> controller_impl_type;

				//controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				// Note: there is nothing to configure

				ptr_controller = ::dcs::make_shared<controller_impl_type>(ptr_dc, controller_conf.sampling_time);
			}
			break;
	}

	ptr_controller->sampling_time(controller_conf.sampling_time);

	return ptr_controller;
}

}} // Namespace detail::<unnamed>


template <
	typename TraitsT,
	typename RealT,
	typename UIntT
>
::dcs::shared_ptr<
	::dcs::eesim::data_center_manager<TraitsT>
> make_data_center_manager(configuration<RealT,UIntT> const& conf,
						   ::dcs::shared_ptr< data_center<TraitsT> > const& ptr_dc)
{
	typedef TraitsT traits_type;
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef ::dcs::eesim::data_center_manager<traits_type> data_center_manager_type;

	::dcs::shared_ptr<data_center_manager_type> ptr_dc_mngr = ::dcs::make_shared<data_center_manager_type>();

	// Controlled data center
	ptr_dc_mngr->controlled_data_center(ptr_dc);

	// Initial placement strategy
	{
		::dcs::shared_ptr< ::dcs::eesim::base_initial_placement_strategy<traits_type> > ptr_strategy;

		ptr_strategy = detail::make_initial_placement_strategy<traits_type>(
							conf.data_center().initial_placement_strategy()
			);

		ptr_dc_mngr->initial_placement_strategy(ptr_strategy);
	}

	// Migration controller
	{
		::dcs::shared_ptr< ::dcs::eesim::base_migration_controller<traits_type> > ptr_controller;

		ptr_controller = detail::make_migration_controller<traits_type>(
							conf.data_center().migration_controller(),
							ptr_dc
			);

		ptr_dc_mngr->migration_controller(ptr_controller);
	}

	return ptr_dc_mngr;
}


}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_DATA_CENTER_MANAGER_HPP
