#ifndef DCS_DES_CLOUD_CONFIG_SIMULATION_HPP
#define DCS_DES_CLOUD_CONFIG_SIMULATION_HPP


#include <boost/variant.hpp>
#include <iosfwd>


namespace dcs { namespace des { namespace cloud { namespace config {

enum output_analysis_category
{
	independent_replications_output_analysis
};


enum num_replications_detector_category
{
	constant_num_replications_detector,
	banks2005_num_replications_detector
};


enum replication_size_detector_category
{
	fixed_duration_replication_size_detector,
	fixed_num_obs_replication_size_detector
};


template <typename UIntT>
struct constant_num_replications_detector_config
{
	typedef UIntT uint_type;

	uint_type num_replications;
};


template <typename RealT, typename UIntT>
struct banks2005_num_replications_detector_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;

//	real_type confidence_level;
//	real_type relative_precision;
	uint_type min_num_replications;
	uint_type max_num_replications;
};


template <typename RealT>
struct fixed_duration_replication_size_detector_config
{
	typedef RealT real_type;

	real_type replication_duration;
};


template <typename UIntT>
struct fixed_num_obs_replication_size_detector_config
{
	typedef UIntT uint_type;

	uint_type num_observations;
};


template <typename RealT, typename UIntT>
struct independent_replications_output_analysis_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef banks2005_num_replications_detector_config<real_type,uint_type> banks2005_num_replications_detector_type;
	typedef constant_num_replications_detector_config<uint_type> constant_num_replications_detector_type;
	typedef fixed_duration_replication_size_detector_config<real_type> fixed_duration_replication_size_detector_type;
	typedef fixed_num_obs_replication_size_detector_config<uint_type> fixed_num_obs_replication_size_detector_type;

	num_replications_detector_category num_replications_category;
	::boost::variant<banks2005_num_replications_detector_type,
					 constant_num_replications_detector_type> num_replications_category_conf;
	replication_size_detector_category replication_size_category;
	::boost::variant<fixed_duration_replication_size_detector_type,
					 fixed_num_obs_replication_size_detector_type> replication_size_category_conf;
};


template <typename RealT, typename UIntT>
struct simulation_output_analysis_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef independent_replications_output_analysis_config<real_type,uint_type> independent_replications_config_type;

	real_type confidence_level;
	real_type relative_precision;
	output_analysis_category category;
	::boost::variant<independent_replications_config_type> category_conf;
};


template <typename RealT, typename UIntT>
struct simulation_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef simulation_output_analysis_config<real_type,uint_type> output_analysis_config_type;

	output_analysis_config_type output_analysis;
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


template <typename CharT, typename CharTraitsT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, constant_num_replications_detector_config<UIntT> const& conf)
{
	os << "<(constant-num-replications-detector)"
	   << " num-replications: " << conf.num_replications
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, banks2005_num_replications_detector_config<RealT,UIntT> const& conf)
{
	os << "<(banks2005-num-replications-detector)"
//	   << " confidence-level: " << conf.confidence_level
//	   << ", relative-precision: " << conf.relative_precision
	   << " min-num-replications: " << conf.min_num_replications
	   << ", max-num-replications: " << conf.max_num_replications
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, fixed_duration_replication_size_detector_config<RealT> const& conf)
{
	os << "<(fixed-fixed-duration-replication-size-detector)"
	   << " duration: " << conf.replication_duration
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, fixed_num_obs_replication_size_detector_config<UIntT> const& conf)
{
	os << "<(fixed-num-observations-replication-size-detector)"
	   << " num-observations: " << conf.num_observations
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, independent_replications_output_analysis_config<RealT,UIntT> const& conf)
{
	os << "<(independent-replications-output-analysis)"
	   << " num-replications: " << conf.num_replications_category_conf
	   << ", replication-size: " << conf.replication_size_category_conf
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, simulation_output_analysis_config<RealT,UIntT> const& conf)
{
	os << "<(output-analysis)"
	   << " confidence-level: " << conf.confidence_level
	   << " relative-precision: " << conf.relative_precision
	   << ", method: " << conf.category_conf
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, simulation_config<RealT,UIntT> const& conf)
{
	os << "<(simulation)"
	   << " output-analysis: " << conf.output_analysis
	   << ">";

	return os;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_SIMULATION_HPP
