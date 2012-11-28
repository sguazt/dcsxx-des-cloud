#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_APPLICATION_CONTROLLER_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_APPLICATION_CONTROLLER_HPP


#include <boost/variant.hpp>
#include <dcs/des/cloud/application_controller_triggers.hpp>
#include <dcs/des/cloud/base_application_controller.hpp>
#include <dcs/des/cloud/config/application_controller.hpp>
#include <dcs/des/cloud/dummy_application_controller.hpp>
#include <dcs/des/cloud/lq_application_controller.hpp>
#include <dcs/des/cloud/qn_application_controller.hpp>
#include <dcs/des/cloud/system_identification_strategy_params.hpp>
#include <dcs/memory.hpp>
#include <stdexcept>


namespace dcs { namespace des { namespace cloud { namespace config {

namespace detail { namespace /*<unnamed>*/ {

template <typename RealT, typename UIntT, typename TraitsT>
void make_common_system_identification_strategy_params(base_rls_system_identification_config<RealT,UIntT> const& ident_conf,
													   ::dcs::des::cloud::rls_system_identification_strategy_params<TraitsT>& params)
{
	params.mimo_as_miso(ident_conf.mimo_as_miso);
	params.max_covariance_heuristic(ident_conf.enable_max_cov_heuristic);
	params.max_covariance_heuristic_max_value(ident_conf.max_cov_heuristic_value);
	params.condition_number_covariance_heuristic(ident_conf.enable_cond_cov_heuristic);
	params.condition_number_covariance_heuristic_trusted_digits(ident_conf.cond_cov_heuristic_trust_digits);
}


template <typename TraitsT, typename ControllerConfigT>
::dcs::shared_ptr<
	::dcs::des::cloud::base_system_identification_strategy_params<TraitsT>
> make_system_identification_strategy_params(ControllerConfigT const& controller_conf)
{
	typedef TraitsT traits_type;
	typedef ControllerConfigT controller_config_type;
	typedef ::dcs::des::cloud::base_system_identification_strategy_params<traits_type> system_identification_strategy_params_type;
	typedef ::dcs::shared_ptr<system_identification_strategy_params_type> system_identification_strategy_params_pointer;

	system_identification_strategy_params_pointer ptr_strategy_params;

	switch (controller_conf.ident_category)
	{
		case rls_bittanti1990_system_identification:
			{
				typedef typename controller_config_type::rls_bittanti1990_system_identification_config_type ident_category_config_impl_type;
				typedef ::dcs::des::cloud::rls_bittanti1990_system_identification_strategy_params<traits_type> system_identification_strategy_params_impl_type;
				typedef ::dcs::shared_ptr<system_identification_strategy_params_impl_type> system_identification_strategy_params_impl_pointer;

				ident_category_config_impl_type const& ident_conf_impl = ::boost::get<ident_category_config_impl_type>(controller_conf.ident_category_conf);
				//ptr_strategy_params = ::dcs::make_shared<system_identification_strategy_params_impl_type>(
				system_identification_strategy_params_impl_pointer ptr_strategy_params_impl(
						new system_identification_strategy_params_impl_type(
								ident_conf_impl.forgetting_factor,
								ident_conf_impl.delta
							)
					);

				make_common_system_identification_strategy_params(ident_conf_impl, *ptr_strategy_params_impl);

				ptr_strategy_params = ptr_strategy_params_impl;
			}
			break;
		case rls_ff_system_identification:
			{
				typedef typename controller_config_type::rls_ff_system_identification_config_type ident_category_config_impl_type;
				typedef ::dcs::des::cloud::rls_ff_system_identification_strategy_params<traits_type> system_identification_strategy_params_impl_type;
				typedef ::dcs::shared_ptr<system_identification_strategy_params_impl_type> system_identification_strategy_params_impl_pointer;

				ident_category_config_impl_type const& ident_conf_impl = ::boost::get<ident_category_config_impl_type>(controller_conf.ident_category_conf);
				//ptr_strategy_params = ::dcs::make_shared<system_identification_strategy_params_impl_type>(
				system_identification_strategy_params_impl_pointer ptr_strategy_params_impl(
						new system_identification_strategy_params_impl_type(
											ident_conf_impl.forgetting_factor
							)
					);

				make_common_system_identification_strategy_params(ident_conf_impl, *ptr_strategy_params_impl);

				ptr_strategy_params = ptr_strategy_params_impl;
			}
			break;
		case rls_kulhavy1984_system_identification:
			{
				typedef typename controller_config_type::rls_kulhavy1984_system_identification_config_type ident_category_config_impl_type;
				typedef ::dcs::des::cloud::rls_kulhavy1984_system_identification_strategy_params<traits_type> system_identification_strategy_params_impl_type;
				typedef ::dcs::shared_ptr<system_identification_strategy_params_impl_type> system_identification_strategy_params_impl_pointer;

				ident_category_config_impl_type const& ident_conf_impl = ::boost::get<ident_category_config_impl_type>(controller_conf.ident_category_conf);
				//ptr_strategy_params = ::dcs::make_shared<system_identification_strategy_params_impl_type>(
				system_identification_strategy_params_impl_pointer ptr_strategy_params_impl(
						new system_identification_strategy_params_impl_type(
											ident_conf_impl.forgetting_factor
							)
					);

				make_common_system_identification_strategy_params(ident_conf_impl, *ptr_strategy_params_impl);

				ptr_strategy_params = ptr_strategy_params_impl;
			}
			break;
		case rls_park1991_system_identification:
			{
				typedef typename controller_config_type::rls_park1991_system_identification_config_type ident_category_config_impl_type;
				typedef ::dcs::des::cloud::rls_park1991_system_identification_strategy_params<traits_type> system_identification_strategy_params_impl_type;
				typedef ::dcs::shared_ptr<system_identification_strategy_params_impl_type> system_identification_strategy_params_impl_pointer;

				ident_category_config_impl_type const& ident_conf_impl = ::boost::get<ident_category_config_impl_type>(controller_conf.ident_category_conf);
				//ptr_strategy_params = ::dcs::make_shared<system_identification_strategy_params_impl_type>(
				system_identification_strategy_params_impl_pointer ptr_strategy_params_impl(
						new system_identification_strategy_params_impl_type(
											ident_conf_impl.forgetting_factor,
											ident_conf_impl.rho
							)
					);

				make_common_system_identification_strategy_params(ident_conf_impl, *ptr_strategy_params_impl);

				ptr_strategy_params = ptr_strategy_params_impl;
			}
			break;
		default:
			throw ::std::runtime_error("[dcs::des::cloud::config::make_data_center] RLS variant not yet implemented.");
	}

	return ptr_strategy_params;
}

}}} // Namespace detail::<unnamed>


template <typename TraitsT, typename RealT, typename UIntT>
::dcs::shared_ptr<
	::dcs::des::cloud::base_application_controller<TraitsT>
> make_application_controller(application_controller_config<RealT,UIntT> const& controller_conf,
							  ::dcs::shared_ptr< ::dcs::des::cloud::multi_tier_application<TraitsT> > const& ptr_app)
{
	typedef TraitsT traits_type;
	typedef ::dcs::des::cloud::base_application_controller<traits_type> controller_type;
	typedef application_controller_config<RealT,UIntT> controller_config_type;
	typedef ::dcs::des::cloud::multi_tier_application<traits_type> application_type;
	typedef ::dcs::shared_ptr<application_type> application_pointer;
	typedef ::dcs::des::cloud::application_controller_triggers<traits_type> application_controller_triggers_type;

	::dcs::shared_ptr<controller_type> ptr_controller;

	application_controller_triggers_type triggers;
	triggers.actual_value_sla_ko(controller_conf.triggers.actual_value_sla_ko_enabled);
	triggers.predicted_value_sla_ko(controller_conf.triggers.predicted_value_sla_ko_enabled);

	switch (controller_conf.category)
	{
		case dummy_application_controller:
			{
				//typedef typename controller_config_type::dummy_controller_config_type controller_config_impl_type;
				typedef ::dcs::des::cloud::dummy_application_controller<traits_type> controller_impl_type;

				//controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				// Note: there is nothing to configure

				ptr_controller = ::dcs::make_shared<controller_impl_type>(ptr_app, controller_conf.sampling_time);
			}
			break;
		case fmpc_application_controller:
			{
				typedef typename controller_config_type::fmpc_controller_config_type controller_config_impl_type;
				typedef ::dcs::des::cloud::fmpc_application_controller<traits_type> controller_impl_type;
				typedef ::dcs::des::cloud::base_system_identification_strategy_params<traits_type> system_identification_strategy_params_type;
				typedef ::dcs::shared_ptr<system_identification_strategy_params_type> system_identification_strategy_params_pointer;

				controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				system_identification_strategy_params_pointer ptr_ident_strategy_params;
				ptr_ident_strategy_params = detail::make_system_identification_strategy_params<traits_type>(controller_conf_impl);

//				application_controller_triggers_type triggers;
//				triggers.actual_value_sla_ko(controller_conf_impl.triggers.actual_value_sla_ko_enabled);
//				triggers.predicted_value_sla_ko(controller_conf_impl.triggers.predicted_value_sla_ko_enabled);

				ptr_controller = ::dcs::shared_ptr<controller_impl_type>(
						new controller_impl_type(
							controller_conf_impl.n_a,
							controller_conf_impl.n_b,
							controller_conf_impl.d,
							make_ublas_matrix(controller_conf_impl.Q),
							make_ublas_matrix(controller_conf_impl.R),
							make_ublas_matrix(controller_conf_impl.Qf),
							make_ublas_vector(controller_conf_impl.xmin),
							make_ublas_vector(controller_conf_impl.xmax),
							make_ublas_vector(controller_conf_impl.umin),
							make_ublas_vector(controller_conf_impl.umax),
							controller_conf_impl.prediction_horizon,
							controller_conf_impl.barrier,
							controller_conf_impl.num_iterations,
							ptr_app,
							controller_conf.sampling_time,
//							controller_conf_impl.rls_forgetting_factor,
							ptr_ident_strategy_params,
							triggers,
							controller_conf_impl.ewma_smoothing_factor
						)
					);
			}
			break;
		case lqi_application_controller:
			{
				typedef typename controller_config_type::lqi_controller_config_type controller_config_impl_type;
				typedef ::dcs::des::cloud::lqi_application_controller<traits_type> controller_impl_type;
				typedef ::dcs::des::cloud::base_system_identification_strategy_params<traits_type> system_identification_strategy_params_type;
				typedef ::dcs::shared_ptr<system_identification_strategy_params_type> system_identification_strategy_params_pointer;

				controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				system_identification_strategy_params_pointer ptr_ident_strategy_params;
				ptr_ident_strategy_params = detail::make_system_identification_strategy_params<traits_type>(controller_conf_impl);

//FIXME: Don't work... Why?!!?
//				ptr_controller = ::dcs::make_shared<controller_impl_type>(
//						controller_conf_impl.n_a,
//						controller_conf_impl.n_b,
//						controller_conf_impl.d,
//						make_ublas_matrix(controller_conf_impl.Q),
//						make_ublas_matrix(controller_conf_impl.R),
//						make_ublas_matrix(controller_conf_impl.N),
//						ptr_app,
//						controller_conf.sampling_time,
//						controller_conf_impl.rls_forgetting_factor,
//						controller_conf_impl.ewma_smoothing_factor
//					);
				ptr_controller = ::dcs::shared_ptr<controller_impl_type>(
						new controller_impl_type(
							controller_conf_impl.n_a,
							controller_conf_impl.n_b,
							controller_conf_impl.d,
							make_ublas_matrix(controller_conf_impl.Q),
							make_ublas_matrix(controller_conf_impl.R),
							make_ublas_matrix(controller_conf_impl.N),
							ptr_app,
							controller_conf.sampling_time,
//							controller_conf_impl.integral_weight,
//							controller_conf_impl.rls_forgetting_factor,
							ptr_ident_strategy_params,
							triggers,
							controller_conf_impl.ewma_smoothing_factor
						)
					);
			}
			break;
//		case lqiy_application_controller:
//			{
//				typedef typename controller_config_type::lqiy_controller_config_type controller_config_impl_type;
//				typedef ::dcs::des::cloud::lqiy_application_controller<traits_type> controller_impl_type;
//				typedef ::dcs::des::cloud::base_system_identification_strategy<traits_type> system_identification_strategy_type;
//				typedef ::dcs::shared_ptr<system_identification_strategy_type> system_identification_strategy_pointer;
//
//				controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);
//
//				system_identification_strategy_pointer ptr_ident_strategy;
//				ptr_ident_strategy = make_system_identification_strategy<traits_type>(controller_conf_impl);
//
//				ptr_controller = ::dcs::shared_ptr<controller_impl_type>(
//						new controller_impl_type(
//							controller_conf_impl.n_a,
//							controller_conf_impl.n_b,
//							controller_conf_impl.d,
//							make_ublas_matrix(controller_conf_impl.Q),
//							make_ublas_matrix(controller_conf_impl.R),
//							make_ublas_matrix(controller_conf_impl.N),
//							ptr_app,
//							controller_conf.sampling_time,
////							controller_conf_impl.integral_weight,
////							controller_conf_impl.rls_forgetting_factor,
//							ptr_ident_strategy_params,
//							controller_conf_impl.ewma_smoothing_factor
//						)
//					);
//			}
//			break;
		case lqr_application_controller:
			{
				typedef typename controller_config_type::lqr_controller_config_type controller_config_impl_type;
				typedef ::dcs::des::cloud::lqr_application_controller<traits_type> controller_impl_type;
				typedef ::dcs::des::cloud::base_system_identification_strategy_params<traits_type> system_identification_strategy_params_type;
				typedef ::dcs::shared_ptr<system_identification_strategy_params_type> system_identification_strategy_params_pointer;

				controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				system_identification_strategy_params_pointer ptr_ident_strategy_params;
				ptr_ident_strategy_params = detail::make_system_identification_strategy_params<traits_type>(controller_conf_impl);

//				application_controller_triggers_type triggers;
//				triggers.actual_value_sla_ko(controller_conf_impl.triggers.actual_value_sla_ko_enabled);
//				triggers.predicted_value_sla_ko(controller_conf_impl.triggers.predicted_value_sla_ko_enabled);

				ptr_controller = ::dcs::shared_ptr<controller_impl_type>(
						new controller_impl_type(
							controller_conf_impl.n_a,
							controller_conf_impl.n_b,
							controller_conf_impl.d,
							make_ublas_matrix(controller_conf_impl.Q),
							make_ublas_matrix(controller_conf_impl.R),
							make_ublas_matrix(controller_conf_impl.N),
							ptr_app,
							controller_conf.sampling_time,
//							controller_conf_impl.rls_forgetting_factor,
							ptr_ident_strategy_params,
							triggers,
							controller_conf_impl.ewma_smoothing_factor
						)
					);
			}
			break;
		case lqry_application_controller:
			{
				typedef typename controller_config_type::lqry_controller_config_type controller_config_impl_type;
				typedef ::dcs::des::cloud::lqry_application_controller<traits_type> controller_impl_type;
				typedef ::dcs::des::cloud::base_system_identification_strategy_params<traits_type> system_identification_strategy_params_type;
				typedef ::dcs::shared_ptr<system_identification_strategy_params_type> system_identification_strategy_params_pointer;

				controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				system_identification_strategy_params_pointer ptr_ident_strategy_params;
				ptr_ident_strategy_params = detail::make_system_identification_strategy_params<traits_type>(controller_conf_impl);

				ptr_controller = ::dcs::shared_ptr<controller_impl_type>(
						new controller_impl_type(
							controller_conf_impl.n_a,
							controller_conf_impl.n_b,
							controller_conf_impl.d,
							make_ublas_matrix(controller_conf_impl.Q),
							make_ublas_matrix(controller_conf_impl.R),
							make_ublas_matrix(controller_conf_impl.N),
							ptr_app,
							controller_conf.sampling_time,
//							controller_conf_impl.rls_forgetting_factor,
							ptr_ident_strategy_params,
							triggers,
							controller_conf_impl.ewma_smoothing_factor
						)
					);
			}
			break;
		case matlab_lqi_application_controller:
			{
				typedef typename controller_config_type::matlab_lqi_controller_config_type controller_config_impl_type;
				typedef ::dcs::des::cloud::matlab_lqi_application_controller<traits_type> controller_impl_type;
				typedef ::dcs::des::cloud::base_system_identification_strategy_params<traits_type> system_identification_strategy_params_type;
				typedef ::dcs::shared_ptr<system_identification_strategy_params_type> system_identification_strategy_params_pointer;

				controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				system_identification_strategy_params_pointer ptr_ident_strategy_params;
				ptr_ident_strategy_params = detail::make_system_identification_strategy_params<traits_type>(controller_conf_impl);

//FIXME: Don't work... Why?!!?
//				ptr_controller = ::dcs::make_shared<controller_impl_type>(
//						controller_conf_impl.n_a,
//						controller_conf_impl.n_b,
//						controller_conf_impl.d,
//						make_ublas_matrix(controller_conf_impl.Q),
//						make_ublas_matrix(controller_conf_impl.R),
//						make_ublas_matrix(controller_conf_impl.N),
//						ptr_app,
//						controller_conf.sampling_time,
//						controller_conf_impl.rls_forgetting_factor,
//						controller_conf_impl.ewma_smoothing_factor
//					);
				ptr_controller = ::dcs::shared_ptr<controller_impl_type>(
						new controller_impl_type(
							controller_conf_impl.n_a,
							controller_conf_impl.n_b,
							controller_conf_impl.d,
							make_ublas_matrix(controller_conf_impl.Q),
							make_ublas_matrix(controller_conf_impl.R),
							make_ublas_matrix(controller_conf_impl.N),
							ptr_app,
							controller_conf.sampling_time,
//							controller_conf_impl.integral_weight,
//							controller_conf_impl.rls_forgetting_factor,
							ptr_ident_strategy_params,
							triggers,
							controller_conf_impl.ewma_smoothing_factor
						)
					);
			}
			break;
		case matlab_lqr_application_controller:
			{
				typedef typename controller_config_type::matlab_lqr_controller_config_type controller_config_impl_type;
				typedef ::dcs::des::cloud::matlab_lqr_application_controller<traits_type> controller_impl_type;
				typedef ::dcs::des::cloud::base_system_identification_strategy_params<traits_type> system_identification_strategy_params_type;
				typedef ::dcs::shared_ptr<system_identification_strategy_params_type> system_identification_strategy_params_pointer;

				controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				system_identification_strategy_params_pointer ptr_ident_strategy_params;
				ptr_ident_strategy_params = detail::make_system_identification_strategy_params<traits_type>(controller_conf_impl);

				ptr_controller = ::dcs::shared_ptr<controller_impl_type>(
						new controller_impl_type(
							controller_conf_impl.n_a,
							controller_conf_impl.n_b,
							controller_conf_impl.d,
							make_ublas_matrix(controller_conf_impl.Q),
							make_ublas_matrix(controller_conf_impl.R),
							make_ublas_matrix(controller_conf_impl.N),
							ptr_app,
							controller_conf.sampling_time,
							ptr_ident_strategy_params,
							triggers,
							controller_conf_impl.ewma_smoothing_factor
						)
					);
			}
			break;
		case matlab_lqry_application_controller:
			{
				typedef typename controller_config_type::matlab_lqry_controller_config_type controller_config_impl_type;
				typedef ::dcs::des::cloud::matlab_lqry_application_controller<traits_type> controller_impl_type;
				typedef ::dcs::des::cloud::base_system_identification_strategy_params<traits_type> system_identification_strategy_params_type;
				typedef ::dcs::shared_ptr<system_identification_strategy_params_type> system_identification_strategy_params_pointer;

				controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				system_identification_strategy_params_pointer ptr_ident_strategy_params;
				ptr_ident_strategy_params = detail::make_system_identification_strategy_params<traits_type>(controller_conf_impl);

				ptr_controller = ::dcs::shared_ptr<controller_impl_type>(
						new controller_impl_type(
							controller_conf_impl.n_a,
							controller_conf_impl.n_b,
							controller_conf_impl.d,
							make_ublas_matrix(controller_conf_impl.Q),
							make_ublas_matrix(controller_conf_impl.R),
							make_ublas_matrix(controller_conf_impl.N),
							ptr_app,
							controller_conf.sampling_time,
							ptr_ident_strategy_params,
							triggers,
							controller_conf_impl.ewma_smoothing_factor
						)
					);
			}
			break;
		case qn_application_controller:
			{
				//typedef typename controller_config_type::qn_controller_config_type controller_config_impl_type;
				typedef ::dcs::des::cloud::qn_application_controller<traits_type> controller_impl_type;

				//controller_config_impl_type const& controller_conf_impl = ::boost::get<controller_config_impl_type>(controller_conf.category_conf);

				// Note: there is nothing to configure

				ptr_controller = ::dcs::make_shared<controller_impl_type>(ptr_app, controller_conf.sampling_time);
			}
			break;
	}

//	//FIXME: maybe useless (see constructors above)
//	ptr_controller->sampling_time(controller_conf.sampling_time);

	return ptr_controller;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_APPLICATION_CONTROLLER_HPP
