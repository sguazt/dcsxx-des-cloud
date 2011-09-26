#ifndef DCS_EESIM_CONFIGURATION_BASED_APPLICATION_INSTANCE_BUILDER_HPP
#define DCS_EESIM_CONFIGURATION_BASED_APPLICATION_INSTANCE_BUILDER_HPP


#include <dcs/debug.hpp>
#include <dcs/eesim/application_instance.hpp>
#include <dcs/eesim/base_application_controller.hpp>
#include <dcs/eesim/base_application_instance_builder.hpp>
#include <dcs/eesim/config/application.hpp>
#include <dcs/eesim/config/operation//make_application.hpp>
#include <dcs/eesim/config/operation//make_application_controller.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/math/stats/function/rand.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT>
class configuration_based_application_instance_builder: public base_application_instance_builder<TraitsT>
{
	private: typedef base_application_instance_builder<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	private: typedef typename traits_type::configuration_type configuration_type;
	public: typedef config::application_config<
						typename configuration_type::real_type,
						typename configuration_type::uint_type> application_config_type;
	private: typedef typename base_type::application_pointer application_pointer;
	private: typedef typename base_type::application_controller_pointer application_controller_pointer;
	private: typedef typename base_type::urng_type urng_type;
	private: typedef typename base_type::application_instance_type application_instance_type;
	private: typedef typename base_type::application_instance_pointer application_instance_pointer;


	/// Default constructor.
	public: configuration_based_application_instance_builder()
	: base_type()
	{
	}


	public: configuration_based_application_instance_builder(application_config_type const& app_conf,
															 uint_type min_num_insts = uint_type(0),
															 uint_type max_num_insts = uint_type(0),
															 uint_type num_prealloc_insts = uint_type(0))
	: base_type(min_num_insts, max_num_insts, num_prealloc_insts),
	  app_conf_(app_conf)
	{
	}


	private: application_instance_pointer do_build(urng_type& rng, real_type clock)
	{
		typedef registry<traits_type> registry_type;

		real_type st(0);
		real_type rt(0);

		while ((st = ::dcs::math::stats::rand(this->start_time_distribution(), rng)) < 0)
		{
			;
		}
		while ((rt = ::dcs::math::stats::rand(this->run_time_distribution(), rng)) < 0)
		{
			;
		}

		registry_type const& reg(registry_type::instance());
		application_pointer ptr_app;
		ptr_app = config::make_application<traits_type>(app_conf_,
														reg.configuration(),
														reg.uniform_random_generator_ptr(),
														reg.des_engine_ptr());
		application_controller_pointer ptr_app_ctrl;
		ptr_app_ctrl = config::make_application_controller<traits_type>(app_conf_.controller,
																		ptr_app);
		
//		application_instance_pointer ptr_instance;
//		ptr_instance = ::dcs::make_shared<application_instance_type>(
//							new application_instance_type(
//									ptr_app,
//									ptr_app_ctrl,
//									clock+st,
//									rt
//							)
//					);
		application_instance_pointer ptr_instance(
							new application_instance_type(
									ptr_app,
									ptr_app_ctrl,
									clock+st,
									rt
							)
					);
		return ptr_instance;
	}


	private: application_config_type app_conf_;
}; // application_instance_builder

}} // Namespace dcs::eesim


#endif // DCS_EESIM_CONFIGURATION_BASED_APPLICATION_INSTANCE_BUILDER_HPP
