#ifndef DCS_EESIM_BASE_APPLICATION_INSTANCE_BUILDER_HPP
#define DCS_EESIM_BASE_APPLICATION_INSTANCE_BUILDER_HPP


#include <dcs/debug.hpp>
#include <dcs/eesim/application_instance.hpp>
#include <dcs/eesim/base_application_controller.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/math/stats/function/rand.hpp>


namespace dcs { namespace eesim {

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
	  num_prealloc_insts_(0)
	{
	}


	public: base_application_instance_builder(uint_type min_num_insts,
											  uint_type max_num_insts,
											  uint_type num_prealloc_insts)
	: min_num_insts_(min_num_insts),
	  max_num_insts_(max_num_insts),
	  num_prealloc_insts_(num_prealloc_insts)
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


	public: application_instance_pointer operator()(urng_type& rng, real_type clock=real_type(0))
	{
		return do_build(rng, clock);
	}


	private: virtual application_instance_pointer do_build(urng_type& urng, real_type clock) = 0;


//	private: application_pointer ptr_app_;
//	private: application_controller_pointer ptr_ctrl_;
	private: uint_type min_num_insts_;
	private: uint_type max_num_insts_;
	private: uint_type num_prealloc_insts_;
	private: distribution_type st_distr_;
	private: distribution_type rt_distr_;
}; // base_application_instance_builder


}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_APPLICATION_INSTANCE_BUILDER_HPP
