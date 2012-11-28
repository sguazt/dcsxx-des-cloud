/**
 * \file dcs/des/cloud/config/application_tier.hpp
 *
 * \brief Configuration for application tier models.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright (C) 2009-2012  Marco Guazzone (marco.guazzone@gmail.com)
 *                          [Distributed Computing System (DCS) Group,
 *                           Computer Science Institute,
 *                           Department of Science and Technological Innovation,
 *                           University of Piemonte Orientale,
 *                           Alessandria (Italy)]
 *
 * This file is part of dcsxx-des-cloud.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DCS_DES_CLOUD_CONFIG_APPLICATION_TIER_HPP
#define DCS_DES_CLOUD_CONFIG_APPLICATION_TIER_HPP


#include <dcs/des/cloud/config/physical_resource.hpp>
#include <iosfwd>
#include <map>
#include <string>


namespace dcs { namespace des { namespace cloud { namespace config {

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

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_APPLICATION_TIER_HPP
