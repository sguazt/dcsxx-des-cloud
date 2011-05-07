#ifndef DCS_EESIM_FIXED_APPLICATION_PERFORMANCE_MODEL_HPP
#define DCS_EESIM_FIXED_APPLICATION_PERFORMANCE_MODEL_HPP


#include <dcs/eesim/base_application_performance_model.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <map>


namespace dcs { namespace eesim {

template <typename TraitsT>
class fixed_application_performance_model: public base_application_performance_model<TraitsT>
{
	private: typedef base_application_performance_model<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	private: typedef ::std::map<performance_measure_category,real_type> measure_map;
	private: typedef ::std::map<uint_type,measure_map> tier_measure_map;


	public: void application_measure(performance_measure_category category, real_type value)
	{
		app_measure_map_[category] = value;
	}


	public: void tier_measure(uint_type tier_id, performance_measure_category category, real_type value)
	{
		tier_measure_map_[tier_id][category] = value;
	}


	private: real_type do_application_measure(performance_measure_category category) const
	{
		return app_measure_map_.at(category);
	}


	private: real_type do_tier_measure(uint_type tier_id, performance_measure_category category) const
	{
		return tier_measure_map_.at(tier_id).at(category);
	}


	private: measure_map app_measure_map_;
	private: tier_measure_map tier_measure_map_;
};

}} // Namespace dcs::eesim


#endif // DCS_FIXED_EESIM_APPLICATION_PERFORMANCE_MODEL_HPP
