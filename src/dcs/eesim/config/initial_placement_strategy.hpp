#ifndef DCS_EESIM_CONFIG_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_EESIM_CONFIG_INITIAL_PLACEMENT_STRATEGY_HPP


#include <boost/variant.hpp>
#include <dcs/macro.hpp>
#include <iostream>


namespace dcs { namespace eesim { namespace config {

enum initial_placement_strategy_category
{
	first_fit_initial_placement_strategy
};


struct first_fit_initial_placement_strategy_config
{
};


struct initial_placement_strategy_config
{
    typedef first_fit_initial_placement_strategy_config first_fit_initial_placement_strategy_config_type;


	initial_placement_strategy_category category;
    ::boost::variant<first_fit_initial_placement_strategy_config_type> category_conf;

};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, initial_placement_strategy_category const& category)
{
	switch (category)
	{
		case first_fit_initial_placement_strategy:
			os << "first-fit";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, first_fit_initial_placement_strategy_config const& strategy)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( strategy );

	os << "<(first-fit-initial-placement)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, initial_placement_strategy_config const& strategy)
{
	os << strategy.category_conf;

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_INITIAL_PLACEMENT_STRATEGY_HPP
