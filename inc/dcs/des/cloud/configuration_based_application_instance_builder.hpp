/**
 * \file dcs/des/cloud/configuration_based_application_instance_builder.hpp
 *
 * \brief Configuration-driver application instance builder.
 *
 * Copyright (C) 2009-2011  Distributed Computing System (DCS) Group, Computer
 * Science Department - University of Piemonte Orientale, Alessandria (Italy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_CLOUD_CONFIGURATION_BASED_APPLICATION_INSTANCE_BUILDER_HPP
#define DCS_DES_CLOUD_CONFIGURATION_BASED_APPLICATION_INSTANCE_BUILDER_HPP


#include <dcs/debug.hpp>
#include <dcs/des/cloud/application_instance.hpp>
#include <dcs/des/cloud/base_application_controller.hpp>
#include <dcs/des/cloud/base_application_instance_builder.hpp>
#include <dcs/des/cloud/config/application.hpp>
#include <dcs/des/cloud/config/operation//make_application.hpp>
#include <dcs/des/cloud/config/operation//make_application_controller.hpp>
#include <dcs/des/cloud/config/operation//make_probability_distribution.hpp>
#include <dcs/des/cloud/multi_tier_application.hpp>
#include <dcs/des/cloud/registry.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <limits>


namespace dcs { namespace des { namespace cloud {

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


	public: configuration_based_application_instance_builder(application_config_type const& app_conf)
	: base_type(),
	  app_conf_(app_conf)
	{
		init();
	}


	public: void application_config(application_config_type const& app_conf)
	{
		app_conf_ = app_conf;

		init();
	}


	private: void init()
	{
		this->min_num_instances(app_conf_.builder.min_num_instances);
		this->max_num_instances(app_conf_.builder.max_num_instances);
		this->num_preallocated_instances(app_conf_.builder.num_preallocated_instances);
		this->preallocated_is_endless(app_conf_.builder.preallocated_is_endless);
		this->start_time_distribution(
				config::make_probability_distribution<traits_type>(app_conf_.builder.arrival_distribution)
			);
		this->run_time_distribution(
				config::make_probability_distribution<traits_type>(app_conf_.builder.runtime_distribution)
			);
	}


	private: application_instance_pointer do_build(urng_type& rng, bool preallocated, real_type clock)
	{
		typedef registry<traits_type> registry_type;

		real_type st(0);
		real_type rt(::std::numeric_limits<real_type>::infinity());

		if (!preallocated)
		{
			while ((st = ::dcs::math::stats::rand(this->start_time_distribution(), rng)) < 0)
			{
				;
			}
		}
		if (!preallocated || !this->preallocated_is_endless())
		{
			while ((rt = ::dcs::math::stats::rand(this->run_time_distribution(), rng)) < 0)
			{
				;
			}
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

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_CONFIGURATION_BASED_APPLICATION_INSTANCE_BUILDER_HPP
