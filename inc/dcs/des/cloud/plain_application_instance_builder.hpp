/**
 * \file dcs/eesim/application_controller.hpp
 *
 * \brief Plain application instance builder.
 *
 * Copyright (C) 2009-2010  Distributed Computing System (DCS) Group, Computer
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

#ifndef DCS_EESIM_PLAIN_APPLICATION_INSTANCE_BUILDER_HPP
#define DCS_EESIM_PLAIN_APPLICATION_INSTANCE_BUILDER_HPP


#include <dcs/debug.hpp>
#include <dcs/eesim/application_instance.hpp>
#include <dcs/eesim/base_application_controller.hpp>
#include <dcs/eesim/base_application_instance_builder.hpp>
#include <dcs/eesim/config/application.hpp>
#include <dcs/eesim/config/operation//make_application.hpp>
#include <dcs/eesim/config/operation//make_application_controller.hpp>
#include <dcs/eesim/config/operation//make_probability_distribution.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <limits>


namespace dcs { namespace eesim {

template <typename TraitsT>
class plain_application_instance_builder: public base_application_instance_builder<TraitsT>
{
	private: typedef base_application_instance_builder<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename base_type::application_pointer application_pointer;
	public: typedef typename base_type::application_controller_pointer application_controller_pointer;
	public: typedef typename base_type::application_instance_type application_instance_type;
	public: typedef typename base_type::application_instance_pointer application_instance_pointer;


	/// Default constructor.
	public: plain_application_instance_builder()
	: base_type()
	{
	}


	public: plain_application_instance_builder(application_pointer const& ptr_app, application_controller_pointer const& ptr_app_ctrl)
	: base_type(),
	  ptr_app_(ptr_app),
	  ptr_app_ctrl_(ptr_app_ctrl)
	{
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
									ptr_app_,
									ptr_app_ctrl_,
									clock+st,
									rt
							)
					);
		return ptr_instance;
	}


	private: application_pointer ptr_app_;
	private: application_controller_pointer ptr_app_ctrl_;
}; // application_instance_builder

}} // Namespace dcs::eesim


#endif // DCS_EESIM_PLAIN_APPLICATION_INSTANCE_BUILDER_HPP
