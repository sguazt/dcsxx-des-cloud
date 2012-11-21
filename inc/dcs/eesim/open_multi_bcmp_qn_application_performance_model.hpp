#ifndef DCS_EESIM_OPEN_MULTI_BCMP_QN_APPLICATION_PERFORMANCE_MODEL_HPP
#define DCS_EESIM_OPEN_MULTI_BCMP_QN_APPLICATION_PERFORMANCE_MODEL_HPP


#include <dcs/eesim/application_performance_model_traits.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/perfeval/qn/open_multi_bcmp_network.hpp>
#include <limits>
#include <stdexcept>


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
			case busy_time_performance_measure:
				throw ::std::runtime_error("[dcs::eesim::open_multi_bcmp_qn_application_performance_model::application_measure] Busy time measure has not been implemented yet.");
			case response_time_performance_measure:
				return model.system_response_time();
			case throughput_performance_measure:
				return model.system_throughput();
			case utilization_performance_measure:
				throw ::std::runtime_error("[dcs::eesim::open_multi_bcmp_qn_application_performance_model::application_measure] Utilization measure not defined for the whole application.");
			case queue_length_performance_measure:
				return model.system_queue_length();
//			case customers_number_performance_measure:
//				return model.system_customers_number();
//			case waiting_time_performance_measure:
//				return model.system_waiting_time();
//			case unknown_performance_measure:
//				break;
		}

		return ::std::numeric_limits<real_type>::quiet_NaN();
	}


	public: static real_type tier_measure(model_type const& model, uint_type tier_id, performance_measure_category category)
	{
		switch (category)
		{
			case busy_time_performance_measure:
				throw ::std::runtime_error("[dcs::eesim::open_multi_bcmp_qn_application_performance_model::tier_measure] Busy time measure has not been implemented yet.");//FIXME
			case response_time_performance_measure:
				return model.station_response_times()(tier_id);
			case throughput_performance_measure:
				return model.station_throughputs()(tier_id);
			case utilization_performance_measure:
				return model.station_utilizations()(tier_id);
			case queue_length_performance_measure:
				return model.station_queue_lengths()(tier_id);
//			case customers_number_performance_measure:
//				return model.station_customers_numbers()(tier_id);
//			case waiting_time_performance_measure:
//				return model.station_waiting_times()(tier_id);
//			case unknown_performance_measure:
//				break;
		}

		return ::std::numeric_limits<real_type>::quiet_NaN();
	}
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_OPEN_MULTI_BCMP_QN_APPLICATION_PERFORMANCE_MODEL_HPP
