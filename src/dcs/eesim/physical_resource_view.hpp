#ifndef DCS_EESIM_PHYSICAL_RESOURCE_VIEW_HPP
#define DCS_EESIM_PHYSICAL_RESOURCE_VIEW_HPP


#include <dcs/assert.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <stdexcept>


namespace dcs { namespace eesim {

template <typename TraitsT>
class physical_resource_view
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;


	public: physical_resource_view()
		: category_(),
		  capacity_(0),
		  threshold_(0)
	{
	}


	public: physical_resource_view(physical_resource_category category, real_type capacity, real_type threshold)
		: category_(category),
		  capacity_(capacity),
		  threshold_(threshold)
	{
		// check preconditions
		assert_check_capacity(capacity_);
		assert_check_threshold(threshold_);
	}


	public: physical_resource_category category() const
	{
		return category_;
	}


	public: real_type capacity() const
	{
		return capacity_;
	}


	public: real_type utilization_threshold() const
	{
		return threshold_;
	}


	protected: void category(physical_resource_category category)
	{
		category_ = category;
	}


	protected: void capacity(real_type c)
	{
		//pre
		assert_check_capacity(c);

		capacity_ = c;
	}


	protected: void utilization_threshold(real_type t)
	{
		threshold_ = t;
	}


	private: static void assert_check_capacity(real_type c)
	{
		// pre: c must be >= 0.
		DCS_ASSERT(
			c >= 0,
			throw ::std::domain_error("[dcs::eesim::physical_resource_view::assert_check_capacity] Input value is out-of-range.")
		);
	}


	private: static void assert_check_threshold(real_type t)
	{
		// pre: t must be in the range [0,1].
		DCS_ASSERT(
			t >= 0 && t <= 1,
			throw ::std::domain_error("[dcs::eesim::physical_resource_view::assert_check_threshold] Input value is out-of-range.")
		);
	}


	private: physical_resource_category category_;
	private: real_type capacity_;
	private: real_type threshold_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_PHYSICAL_RESOURCE_VIEW_HPP
