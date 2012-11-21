#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_MIGRATION_CONTROLLER_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_MIGRATION_CONTROLLER_HPP


//#include <boost/variant.hpp>
#include <dcs/eesim/base_migration_controller.hpp>
#include <dcs/eesim/best_fit_decreasing_migration_controller.hpp>
#include <dcs/eesim/config/migration_controller.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/optimal_solver_params.hpp>
#include <dcs/eesim/dummy_migration_controller.hpp>
#include <dcs/eesim/optimal_migration_controller.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim { namespace config {

template <typename TraitsT, typename RealT>
::dcs::shared_ptr<
	::dcs::eesim::base_migration_controller<TraitsT>
> make_migration_controller(migration_controller_config<RealT> const& controller_conf, ::dcs::shared_ptr< ::dcs::eesim::data_center<TraitsT> > const& ptr_dc)
{
	typedef TraitsT traits_type;
	typedef ::dcs::eesim::base_migration_controller<traits_type> controller_type;
	typedef migration_controller_config<RealT> controller_config_type;
	typedef ::dcs::eesim::data_center<TraitsT> data_center_type;
	typedef ::dcs::shared_ptr<data_center_type> data_center_pointer;

	::dcs::shared_ptr<controller_type> ptr_controller;

	switch (controller_conf.category)
	{
		case best_fit_decreasing_migration_controller:
			{
				typedef typename controller_config_type::best_fit_decreasing_migration_controller_config_type controller_config_impl_type;
				typedef ::dcs::eesim::best_fit_decreasing_migration_controller<traits_type> controller_impl_type;

				//controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				ptr_controller = ::dcs::make_shared<controller_impl_type>(ptr_dc,
																		  controller_conf.sampling_time);
			}
			break;
		case optimal_migration_controller:
			{
				typedef typename controller_config_type::optimal_migration_controller_config_type controller_config_impl_type;
				typedef ::dcs::eesim::optimal_migration_controller<traits_type> controller_impl_type;

				controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				optimal_solver_params<traits_type> params(controller_conf_impl.category,
														  controller_conf_impl.input_method,
														  controller_conf_impl.solver_id,
														  controller_conf_impl.proxy);

				ptr_controller = ::dcs::make_shared<controller_impl_type>(ptr_dc,
																		  controller_conf.sampling_time,
																		  params,
																		  controller_conf_impl.wp,
																		  controller_conf_impl.wm,
																		  controller_conf_impl.ws);
			}
			break;
		case dummy_migration_controller:
			{
				//typedef typename controller_config_type::dummy_migration_controller_config_type controller_config_impl_type;
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

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_MIGRATION_CONTROLLER_HPP
