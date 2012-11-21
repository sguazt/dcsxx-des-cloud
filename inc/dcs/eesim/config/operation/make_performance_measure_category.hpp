#ifndef DCS_EESIM_CONFIG_OPERATION_MAKE_PERFORMANCE_MEASURE_CATEGORY_HPP
#define DCS_EESIM_CONFIG_OPERATION_MAKE_PERFORMANCE_MEASURE_CATEGORY_HPP


#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/config/metric_category.hpp>
#include <stdexcept>


namespace dcs { namespace eesim { namespace config {

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

	throw ::std::runtime_error("[dcs::eesim::config::detail::make_performance_measure_category] Performance measure category not handled.");
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPERATION_MAKE_PERFORMANCE_MEASURE_CATEGORY_HPP
