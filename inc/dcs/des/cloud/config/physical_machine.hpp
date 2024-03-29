#ifndef DCS_DES_CLOUD_CONFIG_PHYSICAL_MACHINE_HPP
#define DCS_DES_CLOUD_CONFIG_PHYSICAL_MACHINE_HPP


#include <algorithm>
#include <dcs/des/cloud/config/physical_machine_controller.hpp>
#include <dcs/des/cloud/config/physical_resource.hpp>
#include <iosfwd>
#include <iterator>
#include <map>
#include <string>


namespace dcs { namespace des { namespace cloud { namespace config {

template <typename RealT>
struct physical_machine_config
{
	typedef RealT real_type;
	typedef ::std::map< physical_resource_category, physical_resource_config<RealT> > resource_container;
	typedef physical_machine_controller_config<real_type> physical_machine_controller_config_type;

	::std::string name;
	resource_container resources;
	physical_machine_controller_config_type controller;
};


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, physical_machine_config<RealT> const& mach)
{
	os << "<(physical_machine)";

	os << " name: " << mach.name;

	os << ", {";
	typedef typename physical_machine_config<RealT>::resource_container::const_iterator iterator;
	iterator begin_it = mach.resources.begin();
	iterator end_it = mach.resources.end();
	for (iterator it = begin_it; it != end_it; ++it)
	{
		if (it != begin_it)
		{
			os << ", ";
		}
		os << it->first << ": " << it->second;
	}
	os << "}";

	os << ", " << mach.controller;

	os << ">";

	return os;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_PHYSICAL_MACHINE_HPP
