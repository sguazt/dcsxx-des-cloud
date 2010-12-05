#ifndef DCS_EESIM_CONFIG_APPLICATION_SLA_HPP
#define DCS_EESIM_CONFIG_APPLICATION_SLA_HPP


#include <boost/variant.hpp>
#include <dcs/eesim/config/metric_category.hpp>
#include <iostream>
#include <map>
#include <string>


namespace dcs { namespace eesim { namespace config {

enum sla_model_category
{
	step_sla_model
};

template <typename RealT>
struct step_sla_model_config
{
	typedef RealT real_type;

	real_type penalty;
	real_type revenue;
};

template <typename RealT>
struct sla_metric_config
{
	typedef RealT real_type;

	real_type value;
	sla_model_category model_type;
	::boost::variant< step_sla_model_config<real_type> > model_conf;
};


template <typename RealT>
struct application_sla_config
{
	typedef RealT real_type;

	typedef ::std::map< metric_category,sla_metric_config<real_type> > metric_container;

	metric_container metrics;
};


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_sla_config<RealT> const& sla)
{
	os << "<(sla)";
	typedef typename  application_sla_config<RealT>::metric_container::const_iterator iterator;
	iterator begin_it = sla.metrics.begin();
	iterator end_it = sla.metrics.end();
	os << " {";
	for (iterator it = begin_it; it != end_it; ++it)
	{
		if (it != begin_it)
		{
			os << ", ";
		}
		os << it->first << ": {";
		os << "value: " << it->second.value;
		switch (it->second.model_type)
		{
			case step_sla_model:
				{
					os << ", <(step)";
					step_sla_model_config<RealT> model_conf = ::boost::get< step_sla_model_config<RealT> >(it->second.model_conf);
					os <<  " revenue: " << model_conf.revenue
					   << ", penalty: " << model_conf.penalty;
					os << ">";
				}
				break;
		}
		os << "}";
	}
	os << "}";
	os << ">";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_APPLICATION_SLA_HPP
