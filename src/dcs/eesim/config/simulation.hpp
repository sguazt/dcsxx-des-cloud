#ifndef DCS_EESIM_CONFIG_SIMULATION_HPP
#define DCS_EESIM_CONFIG_SIMULATION_HPP


#include <boost/variant.hpp>
#include <dcs/eesim/config/metric_category.hpp>
#include <iostream>
#include <map>


namespace dcs { namespace eesim { namespace config {

enum output_analysis_category
{
	independent_replications_output_analysis
};


enum statistic_category
{
	mean_statistic
};


template <typename RealT>
struct statistic_config
{
	typedef RealT real_type;

	statistic_category type;
	real_type precision;
	real_type confidence_level;
};

template <typename RealT, typename UIntT>
struct independent_replications_output_analysis_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef ::std::map< metric_category,statistic_config<real_type> > statistic_container;

	uint_type num_replications;
	real_type replication_duration;
	statistic_container statistics;
};


template <typename RealT, typename UIntT>
struct simulation_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef independent_replications_output_analysis_config<real_type,uint_type> independent_replications_output_analysis_config_type;

	output_analysis_category output_analysis_type;
	::boost::variant<independent_replications_output_analysis_config_type> output_analysis_conf;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, output_analysis_category category)
{
	switch (category)
	{
		case independent_replications_output_analysis:
			os << "independent-replication";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, statistic_category category)
{
	switch (category)
	{
		case mean_statistic:
			os << "mean";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, statistic_config<RealT> const& stat)
{
	os << "<(statistic)"
	   << " type: " << stat.type
	   << ", precision: " << stat.precision
	   << ", confidence-level: " << stat.confidence_level
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, independent_replications_output_analysis_config<RealT,UIntT> const& conf)
{
	os << "<(independent-replications-output-analysis)"
	   << " num-replications: " << conf.num_replications
	   << ", replication-duration: " << conf.replication_duration;

	typedef typename independent_replications_output_analysis_config<RealT,UIntT>::statistic_container::const_iterator iterator;
	iterator begin_it = conf.statistics.begin();
	iterator end_it = conf.statistics.end();
	os << ", {";
	for (iterator it = begin_it; it != end_it; ++it)
	{
		if (it != begin_it)
		{
			os << ", ";
		}
		os << it->first << ": " << it->second;
	}
	os << "}";

	os << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, simulation_config<RealT,UIntT> const& sim)
{
	os << "<(simulation)"
	   << " " << sim.output_analysis_conf
	   << ">";
//	   << " output-analysis: " << sim.output_analysis_type;
/*
	switch (sim.output_analysis_type)
	{
		case independent_replications_output_analysis:
			{
				independent_replications_output_analysis_config<RealT,UIntT> conf;
				conf = ::boost::get< independent_replications_output_analysis_config<RealT,UIntT> >(sim.output_analysis_conf);
				os << conf;
			}
			break;
	}

	os << ">";
*/

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_SIMULATION_HPP
