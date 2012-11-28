/**
 * \file dcs/des/cloud/config/application_performance_model.hpp
 *
 * \brief Configuration for applications performance models.
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

#ifndef DCS_DES_CLOUD_CONFIG_APPLICATION_PERFORMANCE_MODEL_HPP
#define DCS_DES_CLOUD_CONFIG_APPLICATION_PERFORMANCE_MODEL_HPP


#include <algorithm>
#include <boost/variant.hpp>
#include <dcs/des/cloud/config/metric_category.hpp>
#include <dcs/des/cloud/config/numeric_matrix.hpp>
#include <dcs/macro.hpp>
#include <iosfwd>
#include <iterator>
#include <map>
#include <vector>


namespace dcs { namespace des { namespace cloud { namespace config {

enum application_performance_model_category
{
	open_multi_bcmp_qn_model,
	fixed_application_performance_model
};


template <typename RealT, typename UIntT>
struct fixed_application_performance_model_config
{
	typedef RealT real_type;
	typedef RealT uint_type;
	typedef ::std::map<metric_category,real_type> measure_map;
	typedef ::std::map<uint_type,measure_map> tier_measure_map;

	measure_map app_measures;
	tier_measure_map tier_measures;
};


template <typename RealT, typename UIntT>
struct open_multi_bcmp_qn_application_performance_model_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;

	::std::vector<real_type> arrival_rates;
	numeric_matrix<real_type> visit_ratios;
	numeric_matrix<real_type> routing_probabilities;
	numeric_matrix<real_type> service_times;
	::std::vector<uint_type> num_servers;
};

template <typename RealT, typename UIntT>
struct application_performance_model_config
{
	typedef RealT real_type;
	typedef UIntT uint_type;
	typedef fixed_application_performance_model_config<real_type,uint_type> fixed_config_type;
	typedef open_multi_bcmp_qn_application_performance_model_config<real_type,uint_type> open_multi_bcmp_qn_config_type;

	application_performance_model_category category;
	::boost::variant<fixed_config_type,
					 open_multi_bcmp_qn_config_type> category_conf;
};


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, fixed_application_performance_model_config<RealT,UIntT> const& conf)
{
	typedef fixed_application_performance_model_config<RealT,UIntT> config_type;
	typedef typename config_type::measure_map::const_iterator measure_iterator;
	typedef typename config_type::tier_measure_map::const_iterator tier_measure_iterator;

	os << "fixed:";

	measure_iterator meas_end_it;

	os << " overall: {";
	meas_end_it = conf.app_measures.end();
	for (measure_iterator it = conf.app_measures.begin(); it != meas_end_it; ++it)
	{
		os << it->first << ": " << it->second << ",";
	}
	os << "}";

	os << ", tiers: {";
	tier_measure_iterator tier_end_it(conf.tier_measures.end());
	for (tier_measure_iterator tier_it = conf.tier_measures.begin(); tier_it != tier_end_it; ++tier_it)
	{
		os << tier_it->first << " => {";
		meas_end_it = tier_it->second.end();
		for (measure_iterator meas_it = tier_it->second.begin(); meas_it != meas_end_it; ++meas_it)
		{
			os << meas_it->first << ": " << meas_it->second << ",";
		}
		os << "},";
	}
	os << "}";

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, open_multi_bcmp_qn_application_performance_model_config<RealT,UIntT> const& conf)
{
	os << "open-multi-bcmp-qn:"
	   << ", arrival-rates: [";
	::std::copy(conf.arrival_rates.begin(),
				conf.arrival_rates.end(),
				::std::ostream_iterator<RealT>(os, " "));
	os << "]";

	if (!conf.visit_ratios.empty())
	{
		os << ", visit-ratios: " << conf.visit_ratios;
	}
	else if (!conf.routing_probabilities.empty())
	{
		os << ", routing-probabilities: " << conf.routing_probabilities;
	}
	os << ", service-times: " << conf.service_times;

	return os;
}


template <typename CharT, typename CharTraitsT, typename RealT, typename UIntT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, application_performance_model_config<RealT,UIntT> const& model)
{
	typedef application_performance_model_config<RealT,UIntT> model_config_type;

	os << "<(application_performance_model)"
	   << " " << model.category_conf
	   << ">";
 
	return os;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_APPLICATION_PERFORMANCE_MODEL_HPP
