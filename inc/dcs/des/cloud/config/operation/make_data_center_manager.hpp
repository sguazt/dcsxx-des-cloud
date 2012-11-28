#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_DATA_CENTER_MANAGER_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_DATA_CENTER_MANAGER_HPP


//#include <dcs/des/cloud/base_application_instance_builder.hpp>
//#include <dcs/des/cloud/base_incremental_placement_strategy.hpp>
//#include <dcs/des/cloud/base_initial_placement_strategy.hpp>
//#include <dcs/des/cloud/base_migration_controller.hpp>
#include <dcs/des/cloud/config/configuration.hpp>
#include <dcs/des/cloud/config/operation/make_application_instance_builder.hpp>
#include <dcs/des/cloud/config/operation/make_incremental_placement_strategy.hpp>
#include <dcs/des/cloud/config/operation/make_initial_placement_strategy.hpp>
#include <dcs/des/cloud/config/operation/make_migration_controller.hpp>
#include <dcs/des/cloud/data_center.hpp>
#include <dcs/des/cloud/data_center_manager.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace des { namespace cloud { namespace config {

template <
	typename TraitsT,
	typename RealT,
	typename UIntT
>
::dcs::shared_ptr<
	::dcs::des::cloud::data_center_manager<TraitsT>
> make_data_center_manager(configuration<RealT,UIntT> const& conf,
						   ::dcs::shared_ptr< data_center<TraitsT> > const& ptr_dc)
{
	typedef TraitsT traits_type;
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef ::dcs::des::cloud::data_center_manager<traits_type> data_center_manager_type;
	typedef configuration<real_type,uint_type> configuration_type;


	::dcs::shared_ptr<data_center_manager_type> ptr_dc_mngr(new data_center_manager_type());

	// Controlled data center
	ptr_dc_mngr->controlled_data_center(ptr_dc);

	// Initial placement strategy
	ptr_dc_mngr->initial_placement_strategy(
			make_initial_placement_strategy<traits_type>(
				conf.data_center().initial_placement_strategy()
			)
		);

	// Incremental placement strategy
	ptr_dc_mngr->incremental_placement_strategy(
			make_incremental_placement_strategy<traits_type>(
				conf.data_center().incremental_placement_strategy()
			)
		);

	// Migration controller
	ptr_dc_mngr->migration_controller(
			make_migration_controller<traits_type>(
				conf.data_center().migration_controller(),
				ptr_dc
			)
		);

	// Application builders
	{
		typedef typename configuration_type::data_center_config_type data_center_config_type;
		typedef typename data_center_config_type::application_config_container::const_iterator iterator;

		iterator end_it(conf.data_center().applications().end());
		for (iterator it = conf.data_center().applications().begin(); it != end_it; ++it)
		{
//			::dcs::shared_ptr< ::dcs::des::cloud::multi_tier_application<traits_type> > ptr_app;
//
//			ptr_app = make_application<traits_type>(*it, conf, ptr_rng, ptr_des_eng);
//
//			::dcs::shared_ptr< ::dcs::des::cloud::base_application_controller<traits_type> > ptr_app_controller;
//
//			ptr_app_controller = make_application_controller<traits_type>(it->controller, ptr_app);
//			ptr_app_controller->application(ptr_app);
//
//			ptr_dc->add_application(ptr_app, ptr_app_controller);
			ptr_dc_mngr->add_application_instance_builder(
					make_application_instance_builder<traits_type>(*it)
				);
		}
	}

	return ptr_dc_mngr;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_DATA_CENTER_MANAGER_HPP
