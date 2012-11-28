/**
 * \file dcs/des/cloud/config/application.hpp
 *
 * \brief Configuration for applications.
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

#ifndef DCS_DES_CLOUD_CONFIG_APPLICATION_HPP
#define DCS_DES_CLOUD_CONFIG_APPLICATION_HPP


#include <algorithm>
#include <dcs/des/cloud/config/application_builder.hpp>
#include <dcs/des/cloud/config/application_controller.hpp>
#include <dcs/des/cloud/config/application_performance_model.hpp>
#include <dcs/des/cloud/config/application_simulation_model.hpp>
#include <dcs/des/cloud/config/application_sla.hpp>
#include <dcs/des/cloud/config/application_tier.hpp>
#include <dcs/des/cloud/config/physical_resource.hpp>
#include <iosfwd>
#include <iterator>
#include <map>
#include <string>
#include <vector>


namespace dcs { namespace des { namespace cloud { namespace config {

template <typename RealT, typename UIntT>
struct application_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef ::std::map<physical_resource_category,real_type> reference_resource_container;
	typedef application_tier_config<real_type> tier_config_type;
	typedef ::std::vector< application_tier_config<real_type> > tier_container;
	typedef application_performance_model_config<real_type,uint_type> performance_model_config_type;
	typedef application_simulation_model_config<real_type,uint_type> simulation_model_config_type;
	typedef application_sla_config<real_type> sla_config_type;
	typedef application_controller_config<RealT,UIntT> controller_config_type;
	typedef application_builder_config<RealT,UIntT> builder_config_type;

	::std::string name;
	performance_model_config_type perf_model;
	simulation_model_config_type sim_model;
	sla_config_type sla;
	reference_resource_container reference_resources;
	tier_container tiers;
	controller_config_type controller;
	builder_config_type builder;
};


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_config<RealT,UIntT> const& app)
{
	os << "<(application)"
	   <<  " name: " << app.name
	   << ", sla: " << app.sla
	   << ", tiers: [";
	::std::copy(app.tiers.begin(),
				app.tiers.end(),
				::std::ostream_iterator< application_tier_config<RealT> >(os, ", "));

	os << "]";
	os << ", perf-model: " << app.perf_model
	   << ", sim-model: " << app.sim_model;
	typedef typename application_config<RealT,UIntT>::reference_resource_container::const_iterator iterator;
	iterator begin_it = app.reference_resources.begin();
	iterator end_it = app.reference_resources.end();
	os << ", {";
	for (iterator it = begin_it; it != end_it; ++it)
	{
		if (it != begin_it)
		{
			os << ", ";
		}
//		::std::string res;
//		switch (it->first)
//		{
//			case cpu_resource:
//				res = "cpu";
//				break;
//			case mem_resource:
//				res = "mem";
//				break;
//			case disk_resource:
//				res = "disk";
//				break;
//			case nic_resource:
//				res = "nic";
//				break;
//		}
//		os << res << ": " << it->second;
		os << it->first << ": " << it->second;
	}
	os << "}";

	os << ", " << app.controller;
	os << ", " << app.builder;

	os << ">";

	return os;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_APPLICATION_HPP
