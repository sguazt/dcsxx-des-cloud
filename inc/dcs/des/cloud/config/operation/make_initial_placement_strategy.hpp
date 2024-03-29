#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_INITIAL_PLACEMENT_STRATEGY_HPP


//#include <boost/variant.hpp>
#include <dcs/des/cloud/base_initial_placement_strategy.hpp>
#include <dcs/des/cloud/best_fit_decreasing_initial_placement_strategy.hpp>
#include <dcs/des/cloud/best_fit_initial_placement_strategy.hpp>
#include <dcs/des/cloud/config/initial_placement_strategy.hpp>
#include <dcs/des/cloud/first_fit_initial_placement_strategy.hpp>
#include <dcs/des/cloud/first_fit_scaleout_initial_placement_strategy.hpp>
#include <dcs/des/cloud/optimal_initial_placement_strategy.hpp>
#include <dcs/des/cloud/optimal_solver_params.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace des { namespace cloud { namespace config {

template <typename TraitsT, typename RealT>
::dcs::shared_ptr<
	::dcs::des::cloud::base_initial_placement_strategy<TraitsT>
> make_initial_placement_strategy(initial_placement_strategy_config<RealT> const& strategy_conf)
{
	typedef TraitsT traits_type;
	typedef ::dcs::des::cloud::base_initial_placement_strategy<traits_type> strategy_type;
	typedef initial_placement_strategy_config<RealT> strategy_config_type;

	::dcs::shared_ptr<strategy_type> ptr_strategy;

	switch (strategy_conf.category)
	{
		case best_fit_decreasing_initial_placement_strategy:
			{
				//typedef typename strategy_config_type::best_fit_decreasing_initial_placement_strategy_config_type strategy_config_impl_type;
				typedef ::dcs::des::cloud::best_fit_decreasing_initial_placement_strategy<traits_type> strategy_impl_type;

				//strategy_config_impl_type const& strategy_conf_impl = ::boost::get<strategy_config_impl_type>(strategy_conf.category_conf);

				// Note: there is nothing to configure

				ptr_strategy = ::dcs::make_shared<strategy_impl_type>();
			}
			break;
		case best_fit_initial_placement_strategy:
			{
				//typedef typename strategy_config_type::best_fit_initial_placement_strategy_config_type strategy_config_impl_type;
				typedef ::dcs::des::cloud::best_fit_initial_placement_strategy<traits_type> strategy_impl_type;

				//strategy_config_impl_type const& strategy_conf_impl = ::boost::get<strategy_config_impl_type>(strategy_conf.category_conf);

				// Note: there is nothing to configure

				ptr_strategy = ::dcs::make_shared<strategy_impl_type>();
			}
			break;
		case first_fit_initial_placement_strategy:
			{
				//typedef typename strategy_config_type::first_fit_initial_placement_strategy_config_type strategy_config_impl_type;
				typedef ::dcs::des::cloud::first_fit_initial_placement_strategy<traits_type> strategy_impl_type;

				//strategy_config_impl_type const& strategy_conf_impl = ::boost::get<strategy_config_impl_type>(strategy_conf.category_conf);

				// Note: there is nothing to configure

				ptr_strategy = ::dcs::make_shared<strategy_impl_type>();
			}
			break;
		case first_fit_scaleout_initial_placement_strategy:
			{
				//typedef typename strategy_config_type::first_fit_scaleout_initial_placement_strategy_config_type strategy_config_impl_type;
				typedef ::dcs::des::cloud::first_fit_scaleout_initial_placement_strategy<traits_type> strategy_impl_type;

				//strategy_config_impl_type const& strategy_conf_impl = ::boost::get<strategy_config_impl_type>(strategy_conf.category_conf);

				// Note: there is nothing to configure

				ptr_strategy = ::dcs::make_shared<strategy_impl_type>();
			}
			break;
		case optimal_initial_placement_strategy:
			{
				typedef typename strategy_config_type::optimal_initial_placement_strategy_config_type strategy_config_impl_type;
				typedef ::dcs::des::cloud::optimal_initial_placement_strategy<traits_type> strategy_impl_type;

				strategy_config_impl_type const& strategy_conf_impl = ::boost::get<strategy_config_impl_type>(strategy_conf.category_conf);

				optimal_solver_params<traits_type> params(strategy_conf_impl.category,
														  strategy_conf_impl.input_method,
														  strategy_conf_impl.solver_id,
														  strategy_conf_impl.proxy);

				ptr_strategy = ::dcs::make_shared<strategy_impl_type>(params,
																	  strategy_conf_impl.wp,
																	  strategy_conf_impl.ws);
			}
			break;
	}

	ptr_strategy->reference_share_penalty(strategy_conf.ref_penalty);

	return ptr_strategy;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_INITIAL_PLACEMENT_STRATEGY_HPP
