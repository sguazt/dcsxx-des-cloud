#ifndef DCS_EESIM_CONFIG_APPLICATION_PERFORMANCE_MODEL_HPP
#define DCS_EESIM_CONFIG_APPLICATION_PERFORMANCE_MODEL_HPP


#include <algorithm>
#include <boost/variant.hpp>
#include <dcs/eesim/config/metric_category.hpp>
#include <dcs/eesim/config/numeric_matrix.hpp>
#include <dcs/macro.hpp>
#include <iostream>
#include <iterator>
#include <map>
#include <vector>


namespace dcs { namespace eesim { namespace config {

enum application_performance_model_category
{
	open_multi_bcmp_qn_model,
	fixed_application_performance_model
};


template <typename RealT, typename UIntT>
struct fixed_application_performance_model_config
{
	typedef RealT real_type;
	typedef RealT uint_type;
	typedef ::std::map<metric_category,real_type> measure_map;
	typedef ::std::map<uint_type,measure_map> tier_measure_map;

	measure_map app_measures;
	tier_measure_map tier_measures;
};


template <typename RealT, typename UIntT>
struct open_multi_bcmp_qn_application_performance_model_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;

	::std::vector<real_type> arrival_rates;
	numeric_matrix<real_type> visit_ratios;
	numeric_matrix<real_type> routing_probabilities;
	numeric_matrix<real_type> service_times;
	::std::vector<uint_type> num_servers;
};

template <typename RealT, typename UIntT>
struct application_performance_model_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef fixed_application_performance_model_config<real_type,uint_type> fixed_config_type;
	typedef open_multi_bcmp_qn_application_performance_model_config<real_type,uint_type> open_multi_bcmp_qn_config_type;

	application_performance_model_category category;
	::boost::variant<fixed_config_type,
					 open_multi_bcmp_qn_config_type> category_conf;
};


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, fixed_application_performance_model_config<RealT,UIntT> const& conf)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(conf);

	os << "fixed:";
//TODO

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, open_multi_bcmp_qn_application_performance_model_config<RealT,UIntT> const& conf)
{
	os << "open-multi-bcmp-qn:"
	   << ", arrival-rates: [";
	::std::copy(conf.arrival_rates.begin(),
				conf.arrival_rates.end(),
				::std::ostream_iterator<RealT>(os, " "));
	os << "]";

	if (!conf.visit_ratios.empty())
	{
		os << ", visit-ratios: " << conf.visit_ratios;
	}
	else if (!conf.routing_probabilities.empty())
	{
		os << ", routing-probabilities: " << conf.routing_probabilities;
	}
	os << ", service-times: " << conf.service_times;

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_performance_model_config<RealT,UIntT> const& model)
{
	typedef application_performance_model_config<RealT,UIntT> model_config_type;

	os << "<(application_performance_model)"
	   << " " << model.category_conf
	   << ">";
 
	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_APPLICATION_PERFORMANCE_MODEL_HPP
