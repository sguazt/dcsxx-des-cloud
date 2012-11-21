#ifndef DCS_EESIM_CONFIG_OPTIMAL_SOLVER_PROXIES_HPP
#define DCS_EESIM_CONFIG_OPTIMAL_SOLVER_PROXIES_HPP


#include <dcs/eesim/optimal_solver_proxies.hpp>
#include <iosfwd>


namespace dcs { namespace eesim { namespace config {

typedef ::dcs::eesim::optimal_solver_proxies optimal_solver_proxies;
/*
enum optimal_solver_proxies
{
	neos_optimal_solver_proxy,
	none_optimal_solver_proxy
};
*/


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, optimal_solver_proxies proxy)
{
	switch (proxy)
	{
		case neos_optimal_solver_proxy:
			os << "NEOS";
			break;
		case none_optimal_solver_proxy:
			os << "none";
			break;
	}

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_OPTIMAL_SOLVER_PROXIES_HPP
