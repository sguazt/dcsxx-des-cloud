#ifndef OPEN_MULTI_BCMP_QN_APPLICATION_PERFORMANCE_MODEL_HPP
#define OPEN_MULTI_BCMP_QN_APPLICATION_PERFORMANCE_MODEL_HPP


#include <dcs/eesim/application_performance_model_traits.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/perfeval/qn/open_multi_bcmp_network.hpp>
#include <limits>


namespace dcs { namespace eesim {

template <typename TraitsT, typename RealT, typename UIntT>
class application_performance_model_traits<
			TraitsT,
			::dcs::perfeval::qn::open_multi_bcmp_network<RealT,UIntT>
		>
{
	public: typedef TraitsT traits_type;
	public: typedef dcs::perfeval::qn::open_multi_bcmp_network<RealT,UIntT> model_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;


	public: static real_type application_measure(model_type const& model, performance_measure_category category)
	{
		switch (category)
		{
			case response_time_performance_measure:
				return model.system_response_time();
			case throughput_performance_measure:
				return model.system_throughput();
			case unknown_performance_measure:
				break;
		}

		return ::std::numeric_limits<real_type>::quiet_NaN();
	}


	public: static real_type tier_measure(model_type const& model, uint_type tier_id, performance_measure_category category)
	{
		switch (category)
		{
			case response_time_performance_measure:
				return model.station_response_times()(tier_id);
			case throughput_performance_measure:
				return model.station_throughputs()(tier_id);
			case unknown_performance_measure:
				break;
		}

		return ::std::numeric_limits<real_type>::quiet_NaN();
	}
};

}} // Namespace dcs::eesim


#endif // OPEN_MULTI_BCMP_QN_APPLICATION_PERFORMANCE_MODEL_HPP
