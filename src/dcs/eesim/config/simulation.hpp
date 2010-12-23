#ifndef DCS_EESIM_CONFIG_SIMULATION_HPP
#define DCS_EESIM_CONFIG_SIMULATION_HPP


#include <boost/variant.hpp>
#include <iostream>


namespace dcs { namespace eesim { namespace config {

enum output_analysis_category
{
	independent_replications_output_analysis
};


template <typename RealT, typename UIntT>
struct independent_replications_output_analysis_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;

	uint_type num_replications;
	real_type replication_duration;
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


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, independent_replications_output_analysis_config<RealT,UIntT> const& conf)
{
	os << "<(independent-replications-output-analysis)"
	   << " num-replications: " << conf.num_replications
	   << ", replication-duration: " << conf.replication_duration
	   << ">";

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
