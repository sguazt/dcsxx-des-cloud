#ifndef DCS_EESIM_CONFIG_APPLICATION_PERFORMANCE_MODEL_HPP
#define DCS_EESIM_CONFIG_APPLICATION_PERFORMANCE_MODEL_HPP


#include <algorithm>
#include <boost/variant.hpp>
#include <dcs/eesim/config/numeric_matrix.hpp>
#include <iostream>
#include <iterator>
#include <vector>


namespace dcs { namespace eesim { namespace config {

enum application_performance_model_category
{
	open_multi_bcmp_qn_model
};


template <typename RealT, typename UIntT>
struct application_performance_model_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;

	struct open_multi_bcmp_qn_config
	{
		::std::vector<real_type> arrival_rates;
		numeric_matrix<real_type> visit_ratios;
		numeric_matrix<real_type> routing_probabilities;
		numeric_matrix<real_type> service_times;
		::std::vector<uint_type> num_servers;
	};

	application_performance_model_category type;
//	union
//	{
//		open_multi_bcmp_qn_config open_multi_bcmp_qn_conf;
//	};
	::boost::variant<open_multi_bcmp_qn_config> type_conf;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_performance_model_category category)
{
	switch (category)
	{
		case open_multi_bcmp_qn_model:
			os << "open-multi-bcmp-qn";
	}

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_performance_model_config<RealT,UIntT> const& model)
{
	typedef application_performance_model_config<RealT,UIntT> model_config_type;

	os << "<(application_performance_model)"
	   << " type: " << model.type;

	switch (model.type)
	{
		case open_multi_bcmp_qn_model:
			{
				typedef typename model_config_type::open_multi_bcmp_qn_config type_conf_type;
				type_conf_type const& type_conf = ::boost::get<type_conf_type>(model.type_conf);
				os << ", arrival-rates: [";
				::std::copy(type_conf.arrival_rates.begin(),
							type_conf.arrival_rates.end(),
							::std::ostream_iterator<RealT>(os, " "));
				os << "]";

				if (!type_conf.visit_ratios.empty())
				{
					os << ", visit-ratios: " << type_conf.visit_ratios;
				}
				else if (!type_conf.routing_probabilities.empty())
				{
					os << ", routing-probabilities: " << type_conf.routing_probabilities;
				}
				os << ", service-times: " << type_conf.service_times;
			}
			break;
	}

	os << ">";
 
	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_APPLICATION_PERFORMANCE_MODEL_HPP
