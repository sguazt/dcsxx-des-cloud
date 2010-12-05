#ifndef DCS_EESIM_PERFORMANCE_MEASURE_HPP
#define DCS_EESIM_PERFORMANCE_MEASURE_HPP


#include <dcs/eesim/performance_measure_category.hpp>
#include <limits>


namespace dcs { namespace eesim {

template <typename TraitsT>
class performance_measure
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;


	public: performance_measure()
	: category_(unknown_performance_measure),
	  value_(::std::numeric_limits<real_type>::quiet_NaN())
	{
	}

	public: performance_measure(performance_measure_category category, real_type value)
	: category_(category),
	  value_(value)
	{
	}


	public: void category(performance_measure_category x)
	{
		category_ = x;
	}


	public: performance_measure_category category() const
	{
		return category_;
	}


	public: void value(real_type x)
	{
		value_ = x;
	}


	public: real_type value() const
	{
		return value_;
	}


	private: performance_measure_category category_;
	private: real_type value_;
};

}} // Namespace dcs::eesim

#endif // DCS_EESIM_PERFORMANCE_MEASURE_HPP
