#ifndef DCS_EESIM_CONFIG_PROBABILITY_DISTRIBUTION_HPP
#define DCS_EESIM_CONFIG_PROBABILITY_DISTRIBUTION_HPP


#include <boost/variant.hpp>
#include <cstddef>
#include <dcs/eesim/config/numeric_matrix.hpp>
#include <iostream>


namespace dcs { namespace eesim { namespace config {

enum probability_distribution_category
{
	exponential_probability_distribution,
	gamma_probability_distribution,
	normal_probability_distribution,
	map_probability_distribution
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


enum map_probability_distribution_characterization_category
{
	standard_map_characterization,
	casale2009_map_characterization
};


template <typename RealT>
struct standard_map_characterization_config
{
	typedef RealT real_type;

	numeric_matrix<real_type> D0;
	numeric_matrix<real_type> D1;
};

template <typename RealT>
struct casale2009_map_characterization_config
{
	typedef RealT real_type;
	typedef ::std::size_t uint_type;

	uint_type order; // order of the MAP process
	real_type mean;
	real_type id; // index of dispersion
};

template <typename RealT>
struct map_probability_distribution_config
{
	typedef RealT real_type;
	typedef standard_map_characterization_config<real_type> standard_characterization_config_type;
	typedef casale2009_map_characterization_config<real_type> casale2009_characterization_config_type;

	map_probability_distribution_characterization_category characterization_category;
	::boost::variant<standard_characterization_config_type,
					 casale2009_characterization_config_type> characterization_conf;
};


template <typename RealT>
struct probability_distribution_config
{
	typedef RealT real_type;
	typedef exponential_probability_distribution_config<RealT> exponential_distribution_config_type;
	typedef gamma_probability_distribution_config<RealT> gamma_distribution_config_type;
	typedef map_probability_distribution_config<RealT> map_distribution_config_type;
	typedef normal_probability_distribution_config<RealT> normal_distribution_config_type;

	probability_distribution_category category;
	::boost::variant<exponential_distribution_config_type,
					 gamma_distribution_config_type,
					 map_distribution_config_type,
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
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, map_probability_distribution_config<RealT> const& config)
{
	typedef map_probability_distribution_config<RealT> map_distribution_type;

	os << "<(map-distribution)";

	switch (config.characterization_category)
	{
		case standard_map_characterization:
			{
				typedef typename map_distribution_type::standard_characterization_config_type characterization_config_type;

				characterization_config_type const& characterization_conf = ::boost::get<characterization_config_type>(config.characterization_conf);
				os << " characterization: standard"
				   << ", D0: " << characterization_conf.D0
				   << ", D1: " << characterization_conf.D1;
			}
			break;
		case casale2009_map_characterization:
			{
				typedef typename map_distribution_type::casale2009_characterization_config_type characterization_config_type;

				characterization_config_type const& characterization_conf = ::boost::get<characterization_config_type>(config.characterization_conf);
				os << " characterization: casale-2009"
				   << ", order: " << characterization_conf.order
				   << ", mean: " << characterization_conf.mean
				   << ", index-of-dispersion: " << characterization_conf.id;
			}
			break;
	}

	os << ">";

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
