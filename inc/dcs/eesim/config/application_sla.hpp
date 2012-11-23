/**
 * \file dcs/eesim/config/application_sla.hpp
 *
 * \brief Configuration for applications SLA models.
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

#ifndef DCS_EESIM_CONFIG_APPLICATION_SLA_HPP
#define DCS_EESIM_CONFIG_APPLICATION_SLA_HPP


#include <boost/variant.hpp>
#include <dcs/eesim/config/metric_category.hpp>
#include <dcs/eesim/config/statistic.hpp>
#include <dcs/macro.hpp>
#include <iosfwd>
#include <map>
#include <string>


namespace dcs { namespace eesim { namespace config {

enum sla_model_category
{
	step_sla_model,
	none_sla_model
};

template <typename RealT>
struct step_sla_model_config
{
	typedef RealT real_type;

	real_type penalty;
	real_type revenue;
};


template <typename RealT>
struct none_sla_model_config
{
	typedef RealT real_type;
};


template <typename RealT>
struct sla_metric_config
{
	typedef RealT real_type;
	typedef statistic_config<real_type> statistic_config_type;

	real_type value;
	real_type tolerance;
	statistic_config_type statistic;
};


template <typename RealT>
struct application_sla_config
{
	typedef RealT real_type;
	typedef sla_metric_config<real_type> sla_metric_config_type;
	typedef step_sla_model_config<real_type> step_sla_model_config_type;
	typedef none_sla_model_config<real_type> none_sla_model_config_type;
	typedef ::std::map<metric_category,sla_metric_config_type> metric_container;

	sla_model_category category;
	::boost::variant<step_sla_model_config_type,
					 none_sla_model_config_type> category_conf;
	metric_container metrics;
};


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, step_sla_model_config<RealT> const& sla)
{
	os << "<(step-sla-model)"
	   <<  " revenue: " << sla.revenue
	   << ", penalty: " << sla.penalty
	   << ">";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, none_sla_model_config<RealT> const& sla)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(sla);

	os << "<(none-sla-model)>";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_sla_config<RealT> const& sla)
{
	os << "<(sla)";
	typedef typename  application_sla_config<RealT>::metric_container::const_iterator iterator;
	os << " " << sla.category_conf;
	iterator begin_it = sla.metrics.begin();
	iterator end_it = sla.metrics.end();
	os << " {";
	for (iterator it = begin_it; it != end_it; ++it)
	{
		if (it != begin_it)
		{
			os << ", ";
		}
		os << it->first << ": {";
		os << "value: " << it->second.value;
		os << ", tolerance: " << it->second.tolerance;
		os << ", statistic: " << it->second.statistic;
		os << "}";
	}
	os << "}";
	os << ">";

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_APPLICATION_SLA_HPP
