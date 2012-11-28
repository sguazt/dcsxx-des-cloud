/**
 * \file dcs/des/cloud/base_application_instance_builder.hpp
 *
 * \brief Base class for application instance builders.
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

#ifndef DCS_DES_CLOUD_BASE_APPLICATION_INSTANCE_BUILDER_HPP
#define DCS_DES_CLOUD_BASE_APPLICATION_INSTANCE_BUILDER_HPP


#include <dcs/debug.hpp>
#include <dcs/des/cloud/application_instance.hpp>
#include <dcs/des/cloud/base_application_controller.hpp>
#include <dcs/des/cloud/multi_tier_application.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/math/stats/function/rand.hpp>


namespace dcs { namespace des { namespace cloud {

template <typename TraitsT>
class base_application_instance_builder
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef multi_tier_application<traits_type> application_type;
	public: typedef ::dcs::shared_ptr<application_type> application_pointer;
	public: typedef base_application_controller<traits_type> application_controller_type;
	public: typedef ::dcs::shared_ptr<application_controller_type> application_controller_pointer;
	public: typedef ::dcs::math::stats::any_distribution<real_type> distribution_type;
	public: typedef typename traits_type::uniform_random_generator_type urng_type;
	public: typedef application_instance<traits_type> application_instance_type;
	public: typedef ::dcs::shared_ptr<application_instance_type> application_instance_pointer;


	/// Default constructor.
	public: base_application_instance_builder()
	: min_num_insts_(0),
	  max_num_insts_(0),
	  num_prealloc_insts_(0),
	  prealloc_endless_(false)
	{
	}


	public: base_application_instance_builder(uint_type min_num_insts,
											  uint_type max_num_insts,
											  uint_type num_prealloc_insts,
											  bool prealloc_endless)
	: min_num_insts_(min_num_insts),
	  max_num_insts_(max_num_insts),
	  num_prealloc_insts_(num_prealloc_insts),
	  prealloc_endless_(prealloc_endless)
	{
	}


	public: void min_num_instances(uint_type n)
	{
		min_num_insts_ = n;
	}


	public: uint_type min_num_instances() const
	{
		return min_num_insts_;
	}


	public: void max_num_instances(uint_type n)
	{
		max_num_insts_ = n;
	}


	public: uint_type max_num_instances() const
	{
		return max_num_insts_;
	}


	public: void num_preallocated_instances(uint_type n)
	{
		num_prealloc_insts_ = n;
	}


	public: uint_type num_preallocated_instances() const
	{
		return num_prealloc_insts_;
	}


	public: void preallocated_is_endless(bool value)
	{
		prealloc_endless_ = value;
	}


	public: bool preallocated_is_endless() const
	{
		return prealloc_endless_;
	}


	public: void start_time_distribution(distribution_type const& distr)
	{
		st_distr_ = distr;
	}


	public: template <typename DistributionT>
		void start_time_distribution(DistributionT distr)
	{
		st_distr_ = ::dcs::math::stats::make_any_distribution(distr);
	}


	public: void run_time_distribution(distribution_type const& distr)
	{
		rt_distr_ = distr;
	}


	public: template <typename DistributionT>
		void run_time_distribution(DistributionT distr)
	{
		rt_distr_ = ::dcs::math::stats::make_any_distribution(distr);
	}


	public: distribution_type& start_time_distribution()
	{
		return st_distr_;
	}


	public: distribution_type const& start_time_distribution() const
	{
		return st_distr_;
	}


	public: distribution_type& run_time_distribution()
	{
		return rt_distr_;
	}


	public: distribution_type const& run_time_distribution() const
	{
		return rt_distr_;
	}


	public: application_instance_pointer operator()(urng_type& rng, bool preallocated, real_type clock=real_type(0))
	{
		return do_build(rng, preallocated, clock);
	}


	private: virtual application_instance_pointer do_build(urng_type& urng, bool preallocated, real_type clock) = 0;


//	private: application_pointer ptr_app_;
//	private: application_controller_pointer ptr_ctrl_;
	private: uint_type min_num_insts_;
	private: uint_type max_num_insts_;
	private: uint_type num_prealloc_insts_;
	private: bool prealloc_endless_;
	private: distribution_type st_distr_;
	private: distribution_type rt_distr_;
}; // base_application_instance_builder


}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_BASE_APPLICATION_INSTANCE_BUILDER_HPP
