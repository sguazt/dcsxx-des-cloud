#ifndef DCS_EESIM_CONFIG_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_EESIM_CONFIG_INITIAL_PLACEMENT_STRATEGY_HPP


#include <boost/variant.hpp>
#include <dcs/eesim/optimal_solver_categories.hpp>
#include <dcs/eesim/optimal_solver_ids.hpp>
#include <dcs/eesim/optimal_solver_input_methods.hpp>
#include <dcs/eesim/optimal_solver_proxies.hpp>
#include <dcs/macro.hpp>
#include <iosfwd>


namespace dcs { namespace eesim { namespace config {

enum initial_placement_strategy_category
{
	best_fit_initial_placement_strategy,
	best_fit_decreasing_initial_placement_strategy,
	first_fit_initial_placement_strategy,
	first_fit_scaleout_initial_placement_strategy,
	optimal_initial_placement_strategy
};


struct best_fit_initial_placement_strategy_config
{
};


struct best_fit_decreasing_initial_placement_strategy_config
{
};


struct first_fit_initial_placement_strategy_config
{
};


struct first_fit_scaleout_initial_placement_strategy_config
{
};


template <typename RealT>
struct optimal_initial_placement_strategy_config
{
	typedef RealT real_type;

	real_type wp;
	real_type ws;
	optimal_solver_categories category;
	optimal_solver_input_methods input_method;
	optimal_solver_ids solver_id;
	optimal_solver_proxies proxy;
};


template <typename RealT>
struct initial_placement_strategy_config
{
	typedef RealT real_type;
    typedef best_fit_initial_placement_strategy_config best_fit_initial_placement_strategy_config_type;
    typedef best_fit_decreasing_initial_placement_strategy_config best_fit_decreasing_initial_placement_strategy_config_type;
    typedef first_fit_initial_placement_strategy_config first_fit_initial_placement_strategy_config_type;
    typedef first_fit_scaleout_initial_placement_strategy_config first_fit_scaleout_initial_placement_strategy_config_type;
    typedef optimal_initial_placement_strategy_config<real_type> optimal_initial_placement_strategy_config_type;


	initial_placement_strategy_category category;
    ::boost::variant<best_fit_initial_placement_strategy_config_type,
					 best_fit_decreasing_initial_placement_strategy_config_type,
					 first_fit_initial_placement_strategy_config_type,
					 first_fit_scaleout_initial_placement_strategy_config_type,
					 optimal_initial_placement_strategy_config_type> category_conf;
	real_type ref_penalty;
};


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, initial_placement_strategy_category const& category)
{
	switch (category)
	{
		case best_fit_initial_placement_strategy:
			os << "best-fit";
			break;
		case best_fit_decreasing_initial_placement_strategy:
			os << "best-fit-decreasing";
			break;
		case first_fit_initial_placement_strategy:
			os << "first-fit";
			break;
		case first_fit_scaleout_initial_placement_strategy:
			os << "first-fit-scaleout";
			break;
		case optimal_initial_placement_strategy:
			os << "optimal";
			break;
	}

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, best_fit_initial_placement_strategy_config const& strategy)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( strategy );

	os << "<(best-fit-initial-placement)>";

	return os;
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, best_fit_decreasing_initial_placement_strategy_config const& strategy)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( strategy );

	os << "<(best-fit-decreasing-initial-placement)>";

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
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, first_fit_scaleout_initial_placement_strategy_config const& strategy)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( strategy );

	os << "<(first-fit-scaleout-initial-placement)>";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, optimal_initial_placement_strategy_config<RealT> const& strategy)
{
	os << "<(optimal-initial-placement)"
	   << " power-weight: " << strategy.wp
	   << ", sla-weight: " << strategy.ws
	   << ", category: " << strategy.category
	   << ", input: " << strategy.input_method
	   << ", solver: " << strategy.solver_id
	   << ", proxy: " << strategy.proxy
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, initial_placement_strategy_config<RealT> const& strategy)
{
	os << strategy.category_conf;

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_INITIAL_PLACEMENT_STRATEGY_HPP