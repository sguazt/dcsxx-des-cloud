#ifndef DCS_EESIM_CONFIG_METRIC_CATEGORY_HPP
#define DCS_EESIM_CONFIG_METRIC_CATEGORY_HPP


namespace dcs { namespace eesim { namespace config {

enum metric_category
{
	response_time_metric,
	throughput_metric
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, metric_category category)
{
	switch (category)
	{
		case response_time_metric:
			os << "response-time";
			break;
		case throughput_metric:
			os << "throughput";
			break;
	}

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_METRIC_CATEGORY_HPP
