#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PERFORMANCE_MEASURE_CATEGORY_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PERFORMANCE_MEASURE_CATEGORY_HPP


#include <dcs/des/cloud/performance_measure_category.hpp>
#include <dcs/des/cloud/config/metric_category.hpp>
#include <stdexcept>


namespace dcs { namespace des { namespace cloud { namespace config {

performance_measure_category make_performance_measure_category(metric_category category)
{
	switch (category)
	{
		case response_time_metric:
			return response_time_performance_measure;
		case throughput_metric:
			return throughput_performance_measure;
		default:
			break;
	}

	throw ::std::runtime_error("[dcs::des::cloud::config::detail::make_performance_measure_category] Performance measure category not handled.");
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_PERFORMANCE_MEASURE_CATEGORY_HPP
