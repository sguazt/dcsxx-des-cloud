#ifndef DCS_EESIM_CONFIG_STATISTIC_HPP
#define DCS_EESIM_CONFIG_STATISTIC_HPP


#include <boost/variant.hpp>
#include <dcs/des/statistic_categories.hpp>
#include <dcs/macro.hpp>
#include <iosfwd>
#include <stdexcept>


namespace dcs { namespace eesim { namespace config {

enum statistic_category
{
	min_statistic,
	mean_statistic,
	max_statistic,
	quantile_statistic
};


struct max_statistic_config
{
	// empty
};


struct mean_statistic_config
{
	// empty
};


struct min_statistic_config
{
	// empty
};


template <typename RealT>
struct quantile_statistic_config
{
	typedef RealT real_type;

	real_type probability;
};


template <typename RealT>
struct statistic_config
{
	typedef RealT real_type;
	typedef max_statistic_config max_statistic_config_type;
	typedef mean_statistic_config mean_statistic_config_type;
	typedef min_statistic_config min_statistic_config_type;
	typedef quantile_statistic_config<real_type> quantile_statistic_config_type;

	statistic_category category;
	::boost::variant<max_statistic_config_type,
					 mean_statistic_config_type,
					 min_statistic_config_type,
					 quantile_statistic_config_type> category_conf;
};


::dcs::des::statistic_category to_des_statistic_category(statistic_category category)
{
	switch (category)
	{
		case max_statistic:
			return ::dcs::des::max_statistic;
		case mean_statistic:
			return ::dcs::des::mean_statistic;
		case min_statistic:
			return ::dcs::des::min_statistic;
		case quantile_statistic:
			return ::dcs::des::quantile_statistic;
	}

	throw ::std::logic_error("[dcs::eesim::config::to_des_statistic_category] Unknown statistic category.");
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, max_statistic_config const& conf)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(conf);

	os << "max";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, mean_statistic_config const& conf)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(conf);

	os << "mean";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, min_statistic_config const& conf)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(conf);

	os << "min";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, quantile_statistic_config<RealT> const& conf)
{
	os << "quantile: " << conf.probability;

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, statistic_config<RealT> const& conf)
{
	os << "<(statistic)"
	   << " " << conf.category_conf
	   << ">";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_STATISTIC_HPP
