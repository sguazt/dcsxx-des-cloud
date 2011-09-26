#ifndef DCS_EESIM_CONFIG_INCREMENTAL_PLACEMENT_STRATEGY_HPP
#define DCS_EESIM_CONFIG_INCREMENTAL_PLACEMENT_STRATEGY_HPP


#include <boost/variant.hpp>
#include <dcs/macro.hpp>
#include <iosfwd>


namespace dcs { namespace eesim { namespace config {

enum incremental_placement_strategy_category
{
	best_fit_incremental_placement_strategy
};


struct best_fit_incremental_placement_strategy_config
{
};


template <typename RealT>
struct incremental_placement_strategy_config
{
	typedef RealT real_type;
    typedef best_fit_incremental_placement_strategy_config best_fit_incremental_placement_strategy_config_type;


	incremental_placement_strategy_category category;
    ::boost::variant<best_fit_incremental_placement_strategy_config_type> category_conf;
	real_type ref_penalty;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, incremental_placement_strategy_category const& category)
{
	switch (category)
	{
		case best_fit_incremental_placement_strategy:
			os << "best-fit";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, best_fit_incremental_placement_strategy_config const& strategy)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( strategy );

	os << "<(best-fit-incremental-placement)>";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, incremental_placement_strategy_config<RealT> const& strategy)
{
	os << strategy.category_conf;

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_INCREMENTAL_PLACEMENT_STRATEGY_HPP
