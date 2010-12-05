#ifndef DCS_EESIM_CONFIG_PROBABILITY_DISTRIBUTION_HPP
#define DCS_EESIM_CONFIG_PROBABILITY_DISTRIBUTION_HPP


#include <boost/variant.hpp>
#include <iostream>


namespace dcs { namespace eesim { namespace config {

enum probability_distribution_category
{
	exponential_probability_distribution,
	gamma_probability_distribution,
	normal_probability_distribution
};


template <typename RealT>
struct exponential_probability_distribution_config
{
	typedef RealT real_type;

	real_type rate;
};


template <typename RealT>
struct gamma_probability_distribution_config
{
	typedef RealT real_type;

	real_type shape;
	real_type scale;
};


template <typename RealT>
struct normal_probability_distribution_config
{
	typedef RealT real_type;

	real_type mean;
	real_type sd;
};


template <typename RealT>
struct probability_distribution_config
{
	typedef RealT real_type;
	typedef exponential_probability_distribution_config<RealT> exponential_distribution_config_type;
	typedef gamma_probability_distribution_config<RealT> gamma_distribution_config_type;
	typedef normal_probability_distribution_config<RealT> normal_distribution_config_type;

	probability_distribution_category category;
	::boost::variant<exponential_distribution_config_type,
					 gamma_distribution_config_type,
					 normal_distribution_config_type> category_conf;
};


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, exponential_probability_distribution_config<RealT> const& config)
{
	os << "<(exponential-distribution)"
	   << " rate: " << config.rate
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, gamma_probability_distribution_config<RealT> const& config)
{
	os << "<(gamma-distribution)"
	   << " shape: " << config.shape
	   << ", scale: " << config.scale
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, normal_probability_distribution_config<RealT> const& config)
{
	os << "<(normal-distribution)"
	   << " mean: " << config.mean
	   << ", sd: " << config.sd
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, probability_distribution_config<RealT> const& config)
{
	os << config.category_conf;

	return os;
}

}}} // Namespace dcs::eesim::config

#endif // DCS_EESIM_CONFIG_PROBABILITY_DISTRIBUTION_HPP
