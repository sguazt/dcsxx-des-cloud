#ifndef DCS_EESIM_BASE_APPLICATION_PERFORMANCE_MODEL_HPP
#define DCS_EESIM_BASE_APPLICATION_PERFORMANCE_MODEL_HPP


#include <dcs/eesim/performance_measure_category.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT>
class base_application_performance_model
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;


	public: real_type application_measure(performance_measure_category category) const
	{
		return do_application_measure(category);
	}


	public: real_type tier_measure(uint_type tier_id, performance_measure_category category) const
	{
		return do_tier_measure(tier_id, category);
	}


	private: virtual real_type do_application_measure(performance_measure_category category) const = 0;

	private: virtual real_type do_tier_measure(uint_type tier_id, performance_measure_category category) const = 0;
};

}} // Namespace dcs::eesim


#endif // DCS_BASE_EESIM_APPLICATION_PERFORMANCE_MODEL_HPP
