#ifndef DCS_EESIM_CONFIG_APPLICATION_TIER_HPP
#define DCS_EESIM_CONFIG_APPLICATION_TIER_HPP


#include <dcs/eesim/config/physical_resource.hpp>
#include <iosfwd>
#include <map>
#include <string>


namespace dcs { namespace eesim { namespace config {

template <typename RealT>
struct application_tier_config
{
	typedef RealT real_type;
	typedef ::std::map<physical_resource_category,real_type> share_container;

	::std::string name;
	share_container shares;
};


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_tier_config<RealT> const& tier)
{
	os << "<(tier)"
	   << " name: " << tier.name;
	typedef typename application_tier_config<RealT>::share_container::const_iterator iterator;

	os << ", {";
	iterator begin_it = tier.shares.begin();
	iterator end_it = tier.shares.end();
	for (iterator it = begin_it; it != end_it; ++it)
	{
		if (it != begin_it)
		{
			os << ", ";
		}
		os << it->first << ": " << it->second;
	}
	os << "}";
	os << ">";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_APPLICATION_TIER_HPP
