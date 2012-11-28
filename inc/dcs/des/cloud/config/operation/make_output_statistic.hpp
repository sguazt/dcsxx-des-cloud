#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_OUTPUT_STATISTIC_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_OUTPUT_STATISTIC_HPP


#include <boost/variant.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/base_analyzable_statistic.hpp>
#include <dcs/des/max_estimator.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/des/min_estimator.hpp>
#include <dcs/des/quantile_estimator.hpp>
#include <dcs/des/null_transient_detector.hpp>
#include <dcs/des/replications/banks2005_num_replications_detector.hpp>
#include <dcs/des/replications/constant_num_replications_detector.hpp>
#include <dcs/des/replications/dummy_num_replications_detector.hpp>
#include <dcs/des/replications/dummy_replication_size_detector.hpp>
#include <dcs/des/replications/engine.hpp>
#include <dcs/des/replications/fixed_duration_replication_size_detector.hpp>
#include <dcs/des/replications/fixed_num_obs_replication_size_detector.hpp>
#include <dcs/des/statistic_categories.hpp>
#include <dcs/des/cloud/base_application_simulation_model.hpp>
#include <dcs/math/constants.hpp>
#include <dcs/memory.hpp>
#include <stdexcept>


namespace dcs { namespace des { namespace cloud { namespace config {

namespace detail { namespace /*<unnamed>*/ {

/*
template <typename TraitsT, typename RealT, typename UIntT>
::dcs::shared_ptr<
	::dcs::des::base_statistic<
		typename TraitsT::real_type,
		typename TraitsT::uint_type
	>
> make_independent_replications_output_statistic(statistic_config<RealT> const& statistic_conf,
												 independent_replications_output_analysis_config<RealT,UIntT> const& output_analysis_conf,
												 ::dcs::shared_ptr<typename TraitsT::uniform_random_generator_type> const& ptr_rng,
												 ::dcs::shared_ptr<typename TraitsT::des_engine_type> const& ptr_engine,
												 bool primary)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef TraitsT traits_type;
	typedef statistic_config<real_type> statistic_config_type;
	typedef independent_replications_output_analysis_config<real_type,uint_type> output_analysis_config_type;
	typedef configuration<real_type,uint_type> configuration_type;
	typedef simulation_info<traits_type> simulation_info_type;
	typedef typename traits_type::uint_type target_uint_type;
	typedef typename traits_type::real_type target_real_type;
	typedef typename traits_type::des_engine_type des_engine_type;
	typedef ::dcs::des::base_statistic<target_real_type,target_uint_type> output_statistic_type;

	::dcs::shared_ptr<output_statistic_type> ptr_stat;

	switch (statistic_conf.category)
	{
		case mean_statistic:
			{
				typedef ::dcs::des::mean_estimator<target_real_type,target_uint_type> output_statistic_impl_type;
				typedef ::dcs::des::null_transient_detector<target_real_type,target_uint_type> transient_detector_type;
				typedef ::dcs::des::replications::engine<target_real_type,target_uint_type> des_engine_impl_type;

				output_statistic_impl_type stat(statistic_conf.confidence_level);
				transient_detector_type trans_detect;
				target_uint_type max_num_obs(::dcs::math::constants::infinity<target_uint_type>::value);
				des_engine_impl_type* ptr_des_eng(dynamic_cast<des_engine_impl_type*>(ptr_engine.get()));
				target_real_type precision(statistic_conf.precision);

				if (primary)
				{
					switch (output_analysis_conf.num_replications_category)
					{
						case constant_num_replications_detector:
							{
								typedef ::dcs::des::replications::constant_num_replications_detector<target_real_type,target_uint_type> num_replications_detector_type;
								typedef typename output_analysis_config_type::constant_num_replications_detector_type num_replications_detector_config_impl_type;

								num_replications_detector_config_impl_type const& num_replications_detector_conf_impl = ::boost::get<num_replications_detector_config_impl_type>(output_analysis_conf.num_replications_category_conf);

								num_replications_detector_type num_reps_detect(num_replications_detector_conf_impl.num_replications);

								switch (output_analysis_conf.replication_size_category)
								{
									case fixed_duration_replication_size_detector:
										{
											typedef ::dcs::des::replications::fixed_duration_replication_size_detector<
															target_real_type,
															target_uint_type,
															des_engine_type
													> replication_size_detector_type;
											typedef typename output_analysis_config_type::fixed_duration_replication_size_detector_type replication_size_detector_config_impl_type;

											replication_size_detector_config_impl_type const& replication_size_detector_conf_impl = ::boost::get<replication_size_detector_config_impl_type>(output_analysis_conf.replication_size_category_conf);

											replication_size_detector_type rep_size_detect(replication_size_detector_conf_impl.replication_duration,
																						   ptr_engine);

											ptr_stat = ::dcs::des::make_analyzable_statistic(
													stat,
													trans_detect,
													rep_size_detect,
													num_reps_detect,
													*ptr_des_eng,
													precision,
													max_num_obs
												);
										}
										break;
									case fixed_num_obs_replication_size_detector:
										{
											typedef ::dcs::des::replications::fixed_num_obs_replication_size_detector<
															target_real_type,
															target_uint_type
													> replication_size_detector_type;
											typedef typename output_analysis_config_type::fixed_num_obs_replication_size_detector_type replication_size_detector_config_impl_type;

											replication_size_detector_config_impl_type const& replication_size_detector_conf_impl = ::boost::get<replication_size_detector_config_impl_type>(output_analysis_conf.replication_size_category_conf);

											replication_size_detector_type rep_size_detect(replication_size_detector_conf_impl.num_observations);

											ptr_stat = ::dcs::des::make_analyzable_statistic(
													stat,
													trans_detect,
													rep_size_detect,
													num_reps_detect,
													*ptr_des_eng,
													precision,
													max_num_obs
												);
										}
										break;
								} // switch (output_analysis_conf.replication_size_category) ...
							}
							break;
						case banks2005_num_replications_detector:
							{
								typedef ::dcs::des::replications::banks2005_num_replications_detector<target_real_type,target_uint_type> num_replications_detector_type;
								typedef typename output_analysis_config_type::banks2005_num_replications_detector_type num_replications_detector_config_impl_type;

								num_replications_detector_config_impl_type const& num_replications_detector_conf_impl = ::boost::get<num_replications_detector_config_impl_type>(output_analysis_conf.num_replications_category_conf);

								num_replications_detector_type num_reps_detect(num_replications_detector_conf_impl.confidence_level,
																			   num_replications_detector_conf_impl.relative_precision,
																			   num_replications_detector_conf_impl.min_num_replications,
																			   num_replications_detector_conf_impl.max_num_replications);

								switch (output_analysis_conf.replication_size_category)
								{
									case fixed_duration_replication_size_detector:
										{
											typedef ::dcs::des::replications::fixed_duration_replication_size_detector<
															target_real_type,
															target_uint_type,
															des_engine_type
													> replication_size_detector_type;
											typedef typename output_analysis_config_type::fixed_duration_replication_size_detector_type replication_size_detector_config_impl_type;

											replication_size_detector_config_impl_type const& replication_size_detector_conf_impl = ::boost::get<replication_size_detector_config_impl_type>(output_analysis_conf.replication_size_category_conf);

											replication_size_detector_type rep_size_detect(replication_size_detector_conf_impl.replication_duration,
																						   ptr_engine);

											ptr_stat = ::dcs::des::make_analyzable_statistic(
													stat,
													trans_detect,
													rep_size_detect,
													num_reps_detect,
													*ptr_des_eng,
													precision,
													max_num_obs
												);
										}
										break;
									case fixed_num_obs_replication_size_detector:
										{
											typedef ::dcs::des::replications::fixed_num_obs_replication_size_detector<
															target_real_type,
															target_uint_type
													> replication_size_detector_type;
											typedef typename output_analysis_config_type::fixed_num_obs_replication_size_detector_type replication_size_detector_config_impl_type;

											replication_size_detector_config_impl_type const& replication_size_detector_conf_impl = ::boost::get<replication_size_detector_config_impl_type>(output_analysis_conf.replication_size_category_conf);

											replication_size_detector_type rep_size_detect(replication_size_detector_conf_impl.num_observations);

											ptr_stat = ::dcs::des::make_analyzable_statistic(
													stat,
													trans_detect,
													rep_size_detect,
													num_reps_detect,
													*ptr_des_eng,
													precision,
													max_num_obs
												);
										}
										break;
								} // switch (output_analysis_conf.replication_size_category) ...
							}
							break;
					} // switch (output_analysis_conf.num_replications_category) ...
				}
				else
				{
					typedef ::dcs::des::replications::dummy_num_replications_detector<target_real_type,target_uint_type> num_replications_detector_type;
					typedef ::dcs::des::replications::dummy_replication_size_detector<
									target_real_type,
									target_uint_type
							> replication_size_detector_type;

					ptr_stat = ::dcs::des::make_analyzable_statistic(
							stat,
							trans_detect,
							replication_size_detector_type(),
							num_replications_detector_type(),
							*ptr_des_eng,
							precision,
							max_num_obs
						);
				}
			}
			break;
	} // switch (statistic_conf.category) ...

	return ptr_stat;
}


template <typename TraitsT, typename RealT, typename UIntT>
::dcs::shared_ptr<
	::dcs::des::base_statistic<
		typename TraitsT::real_type,
		typename TraitsT::uint_type
	>
> make_output_statistic(statistic_config<RealT> const& statistic_conf,
						simulation_config<RealT,UIntT> const& simulation_conf,
						configuration<RealT,UIntT> const& conf,
						::dcs::shared_ptr<typename TraitsT::uniform_random_generator_type> const& ptr_rng,
						::dcs::shared_ptr<typename TraitsT::des_engine_type> const& ptr_engine,
						bool analyzable, bool primary)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef TraitsT traits_type;
	typedef statistic_config<real_type> statistic_config_type;
	typedef typename traits_type::uint_type target_uint_type;
	typedef typename traits_type::real_type target_real_type;
	typedef ::dcs::des::base_statistic<target_real_type,target_uint_type> output_statistic_type;

	::dcs::shared_ptr<output_statistic_type> ptr_stat;

	if (analyzable)
	{
		switch (conf.simulation().output_analysis_type)
		{
			case independent_replications_output_analysis:
				{
					typedef independent_replications_output_analysis_config<real_type,uint_type> output_analysis_config_impl_type;

					output_analysis_config_impl_type const& output_analysis_conf_impl = ::boost::get<output_analysis_config_impl_type>(simulation_conf.output_analysis_conf);

					ptr_stat = make_independent_replications_output_statistic(statistic_conf, output_analysis_conf_impl, sim_info, primary);
				}
				break;
		}
	}
	else
	{
		switch (statistic_conf.category)
		{
			case mean_statistic:
				{
					typedef ::dcs::des::mean_estimator<target_real_type,target_uint_type> output_statistic_impl_type;

					ptr_stat = ::dcs::make_shared<output_statistic_impl_type>(statistic_conf.confidence_level);
				}
				break;
		}
	}

	return ptr_stat;
}
*/


template <
	typename TraitsT,
	typename StatisticT,
	typename RealT,
	typename UIntT
>
::dcs::shared_ptr<
	::dcs::des::base_statistic<
		typename TraitsT::real_type,
		typename TraitsT::uint_type
	>
> make_independent_replications_output_statistic_impl(StatisticT const& stat,
													  simulation_config<RealT,UIntT> const& simulation_conf,
													  //::dcs::shared_ptr<typename TraitsT::uniform_random_generator_type> const& ptr_rng,
													  ::dcs::shared_ptr<typename TraitsT::des_engine_type> const& ptr_engine,
													  bool primary)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type target_real_type;
	typedef typename traits_type::uint_type target_uint_type;
	typedef ::dcs::des::null_transient_detector<target_real_type,target_uint_type> transient_detector_type;
	typedef ::dcs::des::replications::engine<target_real_type,target_uint_type> des_engine_impl_type;
	typedef simulation_config<RealT,UIntT> simulation_config_type;
	typedef typename simulation_config_type::output_analysis_config_type::independent_replications_config_type output_analysis_config_type;
	typedef typename traits_type::des_engine_type des_engine_type;
	typedef ::dcs::des::base_statistic<target_real_type,target_uint_type> output_statistic_type;
	typedef independent_replications_output_analysis_config<real_type,uint_type> output_analysis_config_impl_type;

	output_analysis_config_impl_type const& output_analysis_conf_impl = ::boost::get<output_analysis_config_impl_type>(simulation_conf.output_analysis.category_conf);

	::dcs::shared_ptr<output_statistic_type> ptr_stat;

	target_real_type confidence_level(simulation_conf.output_analysis.confidence_level);
	target_real_type relative_precision(simulation_conf.output_analysis.relative_precision);
	transient_detector_type trans_detect;
	target_uint_type max_num_obs(::dcs::math::constants::infinity<target_uint_type>::value);
	des_engine_impl_type* ptr_des_eng(dynamic_cast<des_engine_impl_type*>(ptr_engine.get()));

	if (primary)
	{
		switch (output_analysis_conf_impl.num_replications_category)
		{
			case constant_num_replications_detector:
				{
					typedef ::dcs::des::replications::constant_num_replications_detector<target_real_type,target_uint_type> num_replications_detector_type;
					typedef typename output_analysis_config_type::constant_num_replications_detector_type num_replications_detector_config_impl_type;

					num_replications_detector_config_impl_type const& num_replications_detector_conf_impl = ::boost::get<num_replications_detector_config_impl_type>(output_analysis_conf_impl.num_replications_category_conf);

					num_replications_detector_type num_reps_detect(num_replications_detector_conf_impl.num_replications);

					switch (output_analysis_conf_impl.replication_size_category)
					{
						case fixed_duration_replication_size_detector:
							{
								typedef ::dcs::des::replications::fixed_duration_replication_size_detector<
												target_real_type,
												target_uint_type,
												des_engine_type
										> replication_size_detector_type;
								typedef typename output_analysis_config_type::fixed_duration_replication_size_detector_type replication_size_detector_config_impl_type;

								replication_size_detector_config_impl_type const& replication_size_detector_conf_impl = ::boost::get<replication_size_detector_config_impl_type>(output_analysis_conf_impl.replication_size_category_conf);

								replication_size_detector_type rep_size_detect(replication_size_detector_conf_impl.replication_duration,
																			   ptr_engine);

								ptr_stat = ::dcs::des::make_analyzable_statistic(
										stat,
										trans_detect,
										rep_size_detect,
										num_reps_detect,
										*ptr_des_eng,
										relative_precision,
										max_num_obs
									);
							}
							break;
						case fixed_num_obs_replication_size_detector:
							{
								typedef ::dcs::des::replications::fixed_num_obs_replication_size_detector<
												target_real_type,
												target_uint_type
										> replication_size_detector_type;
								typedef typename output_analysis_config_type::fixed_num_obs_replication_size_detector_type replication_size_detector_config_impl_type;

								replication_size_detector_config_impl_type const& replication_size_detector_conf_impl = ::boost::get<replication_size_detector_config_impl_type>(output_analysis_conf_impl.replication_size_category_conf);

								replication_size_detector_type rep_size_detect(replication_size_detector_conf_impl.num_observations);

								ptr_stat = ::dcs::des::make_analyzable_statistic(
										stat,
										trans_detect,
										rep_size_detect,
										num_reps_detect,
										*ptr_des_eng,
										relative_precision,
										max_num_obs
									);
							}
							break;
					} // switch (output_analysis_conf_impl.replication_size_category) ...
				}
				break;
			case banks2005_num_replications_detector:
				{
					typedef ::dcs::des::replications::banks2005_num_replications_detector<target_real_type,target_uint_type> num_replications_detector_type;
					typedef typename output_analysis_config_type::banks2005_num_replications_detector_type num_replications_detector_config_impl_type;

					num_replications_detector_config_impl_type const& num_replications_detector_conf_impl = ::boost::get<num_replications_detector_config_impl_type>(output_analysis_conf_impl.num_replications_category_conf);

					num_replications_detector_type num_reps_detect(confidence_level,
																   relative_precision,
																   num_replications_detector_conf_impl.min_num_replications,
																   num_replications_detector_conf_impl.max_num_replications);

					switch (output_analysis_conf_impl.replication_size_category)
					{
						case fixed_duration_replication_size_detector:
							{
								typedef ::dcs::des::replications::fixed_duration_replication_size_detector<
												target_real_type,
												target_uint_type,
												des_engine_type
										> replication_size_detector_type;
								typedef typename output_analysis_config_type::fixed_duration_replication_size_detector_type replication_size_detector_config_impl_type;

								replication_size_detector_config_impl_type const& replication_size_detector_conf_impl = ::boost::get<replication_size_detector_config_impl_type>(output_analysis_conf_impl.replication_size_category_conf);

								replication_size_detector_type rep_size_detect(replication_size_detector_conf_impl.replication_duration,
																			   ptr_engine);

								ptr_stat = ::dcs::des::make_analyzable_statistic(
										stat,
										trans_detect,
										rep_size_detect,
										num_reps_detect,
										*ptr_des_eng,
										relative_precision,
										max_num_obs
									);
							}
							break;
						case fixed_num_obs_replication_size_detector:
							{
								typedef ::dcs::des::replications::fixed_num_obs_replication_size_detector<
												target_real_type,
												target_uint_type
										> replication_size_detector_type;
								typedef typename output_analysis_config_type::fixed_num_obs_replication_size_detector_type replication_size_detector_config_impl_type;

								replication_size_detector_config_impl_type const& replication_size_detector_conf_impl = ::boost::get<replication_size_detector_config_impl_type>(output_analysis_conf_impl.replication_size_category_conf);

								replication_size_detector_type rep_size_detect(replication_size_detector_conf_impl.num_observations);

								ptr_stat = ::dcs::des::make_analyzable_statistic(
										stat,
										trans_detect,
										rep_size_detect,
										num_reps_detect,
										*ptr_des_eng,
										relative_precision,
										max_num_obs
									);
							}
							break;
					} // switch (output_analysis_conf_impl.replication_size_category) ...
				}
				break;
		} // switch (output_analysis_conf_impl.num_replications_category) ...
	}
	else
	{
//		typedef ::dcs::des::replications::dummy_num_replications_detector<target_real_type,target_uint_type> num_replications_detector_type;
//		typedef ::dcs::des::replications::dummy_replication_size_detector<
//						target_real_type,
//						target_uint_type
//				> replication_size_detector_type;
//
//		ptr_stat = ::dcs::des::make_analyzable_statistic(
//				stat,
//				trans_detect,
//				replication_size_detector_type(),
//				num_replications_detector_type(),
//				*ptr_des_eng,
//				::dcs::math::constants::infinity<real_type>::value,
//				max_num_obs
//			);
		ptr_stat = ptr_des_eng->make_analyzable_statistic(stat);
	}

	return ptr_stat;
}


template <
	typename TraitsT,
	typename RealT,
	typename UIntT
>
::dcs::shared_ptr<
	::dcs::des::base_statistic<
		typename TraitsT::real_type,
		typename TraitsT::uint_type
	>
//> make_independent_replications_output_statistic(::dcs::des::statistic_category stat_category, 
> make_independent_replications_output_statistic(statistic_config<RealT> stat_conf, 
												 simulation_config<RealT,UIntT> const& simulation_conf,
												 //::dcs::shared_ptr<typename TraitsT::uniform_random_generator_type> const& ptr_rng,
												 ::dcs::shared_ptr<typename TraitsT::des_engine_type> const& ptr_engine,
												 bool primary)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef TraitsT traits_type;
	typedef statistic_config<RealT> statistic_config_type;
//	typedef simulation_config<RealT,UIntT> simulation_config_type;
//	typedef typename simulation_config_type::output_analysis_config_type::independent_replications_config_type output_analysis_config_type;
//	typedef simulation_info<traits_type> simulation_info_type;
	typedef typename traits_type::uint_type target_uint_type;
	typedef typename traits_type::real_type target_real_type;
//	typedef typename traits_type::des_engine_type des_engine_type;
	typedef ::dcs::des::base_statistic<target_real_type,target_uint_type> output_statistic_type;

//	typedef independent_replications_output_analysis_config<real_type,uint_type> output_analysis_config_impl_type;

//	output_analysis_config_impl_type const& output_analysis_conf_impl = ::boost::get<output_analysis_config_impl_type>(simulation_conf.output_analysis.category_conf);

	::dcs::shared_ptr<output_statistic_type> ptr_stat;

	switch (stat_conf.category)
	{
		case max_statistic:
			{
				typedef ::dcs::des::max_estimator<target_real_type,target_uint_type> output_statistic_impl_type;

				target_real_type confidence_level(simulation_conf.output_analysis.confidence_level);
				output_statistic_impl_type stat(confidence_level);
				ptr_stat = make_independent_replications_output_statistic_impl<traits_type>(stat, simulation_conf, /*ptr_rng,*/ ptr_engine, primary);
			}
			break;
		case mean_statistic:
			{
				typedef ::dcs::des::mean_estimator<target_real_type,target_uint_type> output_statistic_impl_type;
/*
				typedef ::dcs::des::null_transient_detector<target_real_type,target_uint_type> transient_detector_type;
				typedef ::dcs::des::replications::engine<target_real_type,target_uint_type> des_engine_impl_type;

				target_real_type confidence_level(simulation_conf.output_analysis.confidence_level);
				target_real_type relative_precision(simulation_conf.output_analysis.relative_precision);
				transient_detector_type trans_detect;
				target_uint_type max_num_obs(::dcs::math::constants::infinity<target_uint_type>::value);
				des_engine_impl_type* ptr_des_eng(dynamic_cast<des_engine_impl_type*>(ptr_engine.get()));
				output_statistic_impl_type stat(confidence_level);

				if (primary)
				{
					switch (output_analysis_conf_impl.num_replications_category)
					{
						case constant_num_replications_detector:
							{
								typedef ::dcs::des::replications::constant_num_replications_detector<target_real_type,target_uint_type> num_replications_detector_type;
								typedef typename output_analysis_config_type::constant_num_replications_detector_type num_replications_detector_config_impl_type;

								num_replications_detector_config_impl_type const& num_replications_detector_conf_impl = ::boost::get<num_replications_detector_config_impl_type>(output_analysis_conf_impl.num_replications_category_conf);

								num_replications_detector_type num_reps_detect(num_replications_detector_conf_impl.num_replications);

								switch (output_analysis_conf_impl.replication_size_category)
								{
									case fixed_duration_replication_size_detector:
										{
											typedef ::dcs::des::replications::fixed_duration_replication_size_detector<
															target_real_type,
															target_uint_type,
															des_engine_type
													> replication_size_detector_type;
											typedef typename output_analysis_config_type::fixed_duration_replication_size_detector_type replication_size_detector_config_impl_type;

											replication_size_detector_config_impl_type const& replication_size_detector_conf_impl = ::boost::get<replication_size_detector_config_impl_type>(output_analysis_conf_impl.replication_size_category_conf);

											replication_size_detector_type rep_size_detect(replication_size_detector_conf_impl.replication_duration,
																						   ptr_engine);

											ptr_stat = ::dcs::des::make_analyzable_statistic(
													stat,
													trans_detect,
													rep_size_detect,
													num_reps_detect,
													*ptr_des_eng,
													relative_precision,
													max_num_obs
												);
										}
										break;
									case fixed_num_obs_replication_size_detector:
										{
											typedef ::dcs::des::replications::fixed_num_obs_replication_size_detector<
															target_real_type,
															target_uint_type
													> replication_size_detector_type;
											typedef typename output_analysis_config_type::fixed_num_obs_replication_size_detector_type replication_size_detector_config_impl_type;

											replication_size_detector_config_impl_type const& replication_size_detector_conf_impl = ::boost::get<replication_size_detector_config_impl_type>(output_analysis_conf_impl.replication_size_category_conf);

											replication_size_detector_type rep_size_detect(replication_size_detector_conf_impl.num_observations);

											ptr_stat = ::dcs::des::make_analyzable_statistic(
													stat,
													trans_detect,
													rep_size_detect,
													num_reps_detect,
													*ptr_des_eng,
													relative_precision,
													max_num_obs
												);
										}
										break;
								} // switch (output_analysis_conf_impl.replication_size_category) ...
							}
							break;
						case banks2005_num_replications_detector:
							{
								typedef ::dcs::des::replications::banks2005_num_replications_detector<target_real_type,target_uint_type> num_replications_detector_type;
								typedef typename output_analysis_config_type::banks2005_num_replications_detector_type num_replications_detector_config_impl_type;

								num_replications_detector_config_impl_type const& num_replications_detector_conf_impl = ::boost::get<num_replications_detector_config_impl_type>(output_analysis_conf_impl.num_replications_category_conf);

								num_replications_detector_type num_reps_detect(confidence_level,
																			   relative_precision,
																			   num_replications_detector_conf_impl.min_num_replications,
																			   num_replications_detector_conf_impl.max_num_replications);

								switch (output_analysis_conf_impl.replication_size_category)
								{
									case fixed_duration_replication_size_detector:
										{
											typedef ::dcs::des::replications::fixed_duration_replication_size_detector<
															target_real_type,
															target_uint_type,
															des_engine_type
													> replication_size_detector_type;
											typedef typename output_analysis_config_type::fixed_duration_replication_size_detector_type replication_size_detector_config_impl_type;

											replication_size_detector_config_impl_type const& replication_size_detector_conf_impl = ::boost::get<replication_size_detector_config_impl_type>(output_analysis_conf_impl.replication_size_category_conf);

											replication_size_detector_type rep_size_detect(replication_size_detector_conf_impl.replication_duration,
																						   ptr_engine);

											ptr_stat = ::dcs::des::make_analyzable_statistic(
													stat,
													trans_detect,
													rep_size_detect,
													num_reps_detect,
													*ptr_des_eng,
													relative_precision,
													max_num_obs
												);
										}
										break;
									case fixed_num_obs_replication_size_detector:
										{
											typedef ::dcs::des::replications::fixed_num_obs_replication_size_detector<
															target_real_type,
															target_uint_type
													> replication_size_detector_type;
											typedef typename output_analysis_config_type::fixed_num_obs_replication_size_detector_type replication_size_detector_config_impl_type;

											replication_size_detector_config_impl_type const& replication_size_detector_conf_impl = ::boost::get<replication_size_detector_config_impl_type>(output_analysis_conf_impl.replication_size_category_conf);

											replication_size_detector_type rep_size_detect(replication_size_detector_conf_impl.num_observations);

											ptr_stat = ::dcs::des::make_analyzable_statistic(
													stat,
													trans_detect,
													rep_size_detect,
													num_reps_detect,
													*ptr_des_eng,
													relative_precision,
													max_num_obs
												);
										}
										break;
								} // switch (output_analysis_conf_impl.replication_size_category) ...
							}
							break;
					} // switch (output_analysis_conf_impl.num_replications_category) ...
				}
				else
				{
					typedef ::dcs::des::replications::dummy_num_replications_detector<target_real_type,target_uint_type> num_replications_detector_type;
					typedef ::dcs::des::replications::dummy_replication_size_detector<
									target_real_type,
									target_uint_type
							> replication_size_detector_type;

					ptr_stat = ::dcs::des::make_analyzable_statistic(
							stat,
							trans_detect,
							replication_size_detector_type(),
							num_replications_detector_type(),
							*ptr_des_eng,
							::dcs::math::constants::infinity<real_type>::value,
							max_num_obs
						);
				}
*/
				target_real_type confidence_level(simulation_conf.output_analysis.confidence_level);
				output_statistic_impl_type stat(confidence_level);
				ptr_stat = make_independent_replications_output_statistic_impl<traits_type>(stat, simulation_conf, /*ptr_rng,*/ ptr_engine, primary);
			}
			break;
		case min_statistic:
			{
				typedef ::dcs::des::min_estimator<target_real_type,target_uint_type> output_statistic_impl_type;

				target_real_type confidence_level(simulation_conf.output_analysis.confidence_level);
				output_statistic_impl_type stat(confidence_level);
				ptr_stat = make_independent_replications_output_statistic_impl<traits_type>(stat, simulation_conf, /*ptr_rng,*/ ptr_engine, primary);
			}
			break;
		case quantile_statistic:
			{
				typedef ::dcs::des::quantile_estimator<target_real_type,target_uint_type> output_statistic_impl_type;
				typedef typename statistic_config_type::quantile_statistic_config_type statistic_config_impl_type;

				target_real_type confidence_level(simulation_conf.output_analysis.confidence_level);

				statistic_config_impl_type const& stat_conf_impl(::boost::get<statistic_config_impl_type>(stat_conf.category_conf));

				output_statistic_impl_type stat(stat_conf_impl.probability, confidence_level);

				ptr_stat = make_independent_replications_output_statistic_impl<traits_type>(stat, simulation_conf, /*ptr_rng,*/ ptr_engine, primary);
			}
			break;
		default:
			throw ::std::runtime_error("[dcs::des::cloud::config::detail::make_independent_replications_output_statistic] Statistic type not hanlded.");
	} // switch (stat_category) ...

	return ptr_stat;
}

}}} // Namespace detail::<unnamed>


template <
	typename TraitsT,
	typename RealT,
	typename UIntT
>
::dcs::shared_ptr<
	::dcs::des::base_statistic<
		typename TraitsT::real_type,
		typename TraitsT::uint_type
	>
> make_output_statistic(statistic_config<RealT> const& stat_conf,
						simulation_config<RealT,UIntT> const& simulation_conf,
						//::dcs::shared_ptr<typename TraitsT::uniform_random_generator_type> const& ptr_rng,
						::dcs::shared_ptr<typename TraitsT::des_engine_type> const& ptr_engine,
						bool analyzable,
						bool primary)
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef TraitsT traits_type;
	typedef typename traits_type::uint_type target_uint_type;
	typedef typename traits_type::real_type target_real_type;
	typedef ::dcs::des::base_statistic<target_real_type,target_uint_type> output_statistic_type;
	typedef statistic_config<real_type> statistic_config_type;

	::dcs::shared_ptr<output_statistic_type> ptr_stat;

	if (analyzable)
	{
		switch (simulation_conf.output_analysis.category)
		{
			case independent_replications_output_analysis:
				{
					ptr_stat = detail::make_independent_replications_output_statistic<traits_type>(
								stat_conf,
								simulation_conf,
								//ptr_rng,
								ptr_engine,
								primary
						);
				}
				break;
		}
	}
	else
	{
		switch (simulation_conf.output_analysis.category)
		{
			case independent_replications_output_analysis:
				{
					ptr_stat = detail::make_independent_replications_output_statistic<traits_type>(
								stat_conf,
								simulation_conf,
								//ptr_rng,
								ptr_engine,
								false
						);
				}
				break;
		}
/*
		switch (to_des_statistic_category(stat_conf.category))
		{
			case ::dcs::des::max_statistic:
				{
					typedef ::dcs::des::max_estimator<target_real_type,target_uint_type> output_statistic_impl_type;

					ptr_stat = ::dcs::make_shared<output_statistic_impl_type>(simulation_conf.output_analysis.confidence_level);
				}
				break;
			case ::dcs::des::mean_statistic:
				{
					typedef ::dcs::des::mean_estimator<target_real_type,target_uint_type> output_statistic_impl_type;

					ptr_stat = ::dcs::make_shared<output_statistic_impl_type>(simulation_conf.output_analysis.confidence_level);
				}
				break;
			case ::dcs::des::min_statistic:
				{
					typedef ::dcs::des::min_estimator<target_real_type,target_uint_type> output_statistic_impl_type;

					ptr_stat = ::dcs::make_shared<output_statistic_impl_type>(simulation_conf.output_analysis.confidence_level);
				}
				break;
			case ::dcs::des::quantile_statistic:
				{
					typedef ::dcs::des::quantile_estimator<target_real_type,target_uint_type> output_statistic_impl_type;
					typedef typename statistic_config_type::quantile_statistic_config_type statistic_config_impl_type;

					statistic_config_impl_type const& stat_conf_impl = ::boost::get<statistic_config_impl_type>(stat_conf.category_conf);
					ptr_stat = ::dcs::make_shared<output_statistic_impl_type>(stat_conf_impl.probability,
																			  simulation_conf.output_analysis.confidence_level);
				}
				break;
			default:
				throw ::std::runtime_error("[dcs::des::cloud::config::detail::make_output_statistic] Statistic type not hanlded.");
		}
*/
	}

	return ptr_stat;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_OUTPUT_STATISTIC_HPP
