#ifndef DCS_EESIM_CONFIG_PROBABILITY_DISTRIBUTION_HPP
#define DCS_EESIM_CONFIG_PROBABILITY_DISTRIBUTION_HPP


#include <algorithm>
#include <boost/variant.hpp>
#include <cstddef>
#include <dcs/eesim/config/numeric_matrix.hpp>
#include <dcs/functional/bind.hpp>
#include <iostream>
#include <iterator>
#include <utility>


namespace dcs { namespace eesim { namespace config {

enum probability_distribution_category
{
	degenerate_probability_distribution,
	exponential_probability_distribution,
	gamma_probability_distribution,
	map_probability_distribution,
	mmpp_probability_distribution,
	normal_probability_distribution,
	pmpp_probability_distribution,
	timed_step_probability_distribution
};


template <typename RealT>
struct degenerate_probability_distribution_config
{
	typedef RealT real_type;

	real_type k;
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
struct mmpp_probability_distribution_config
{
	typedef RealT real_type;

	numeric_matrix<real_type> Q;
	::std::vector<real_type> rates;
};


template <typename RealT>
struct normal_probability_distribution_config
{
	typedef RealT real_type;

	real_type mean;
	real_type sd;
};


template <typename RealT>
struct pmpp_probability_distribution_config
{
	typedef RealT real_type;
	typedef ::std::vector<real_type> rate_container;

	rate_container rates;
	real_type shape;
	real_type min;
};

template <typename RealT>
struct probability_distribution_config;

template <typename RealT>
struct timed_step_probability_distribution_config
{
	typedef RealT real_type;
	typedef probability_distribution_config<real_type> distribution_type;
	typedef ::std::pair<real_type,dcs::shared_ptr<distribution_type> > phase_type;
	typedef ::std::vector<phase_type> phase_container;

	phase_container phases;
};


template <typename RealT>
struct probability_distribution_config
{
	typedef RealT real_type;
	typedef degenerate_probability_distribution_config<RealT> degenerate_distribution_config_type;
	typedef exponential_probability_distribution_config<RealT> exponential_distribution_config_type;
	typedef gamma_probability_distribution_config<RealT> gamma_distribution_config_type;
	typedef map_probability_distribution_config<RealT> map_distribution_config_type;
	typedef mmpp_probability_distribution_config<RealT> mmpp_distribution_config_type;
	typedef normal_probability_distribution_config<RealT> normal_distribution_config_type;
	typedef pmpp_probability_distribution_config<RealT> pmpp_distribution_config_type;
	typedef timed_step_probability_distribution_config<RealT> timed_step_distribution_config_type;

	probability_distribution_category category;
	::boost::variant<degenerate_distribution_config_type,
					 exponential_distribution_config_type,
					 gamma_distribution_config_type,
					 map_distribution_config_type,
					 mmpp_distribution_config_type,
					 normal_distribution_config_type,
					 pmpp_distribution_config_type,
					 timed_step_distribution_config_type> category_conf;
};


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, degenerate_probability_distribution_config<RealT> const& config)
{
	os << "<(degenerate-distribution)"
	   << " k: " << config.k
	   << ">";

	return os;
}


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
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, mmpp_probability_distribution_config<RealT> const& config)
{
	os << "<(mmpp-distribution)"
	   << " Q: " << config.Q
	   << ", rates: ";
	::std::copy(config.rates.begin(),
				config.rates.end(),
				::std::ostream_iterator<RealT>(os, ", "));
	os << ">";

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
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, pmpp_probability_distribution_config<RealT> const& config)
{
	os << "<(pmpp-distribution)"
	   << " rates: ";
	::std::copy(config.rates.begin(),
				config.rates.end(),
				::std::ostream_iterator<RealT>(os, ", "));
	os << ", shape: " << config.shape
	   << ", min: " << config.min
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, timed_step_probability_distribution_config<RealT> const& config)
{
	typedef typename timed_step_probability_distribution_config<RealT>::phase_container::const_iterator iterator;

	os << "<(timed-step-distribution)"
	   << " phases: ";
	iterator end_it(config.phases.end());
	for (iterator it = config.phases.begin(); it != end_it; ++it)
	{
		os << "{" << it->first << " -> " << *(it->second) << "}";
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
