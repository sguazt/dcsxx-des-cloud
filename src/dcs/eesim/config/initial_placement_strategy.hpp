#ifndef DCS_EESIM_CONFIG_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_EESIM_CONFIG_INITIAL_PLACEMENT_STRATEGY_HPP


#include <iostream>


namespace dcs { namespace eesim { namespace config {

enum initial_placement_strategy_category
{
	first_fit_initial_placement_strategy
};


struct first_fit_initial_placement_strategy_config
{
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
	os << "<(first-fit-initial-placement)>";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_INITIAL_PLACEMENT_STRATEGY_HPP
