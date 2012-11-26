/**
 * \file dcs/eesim/config/metric_category.hpp
 *
 * \brief Configuration for metric categories.
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

#ifndef DCS_EESIM_CONFIG_METRIC_CATEGORY_HPP
#define DCS_EESIM_CONFIG_METRIC_CATEGORY_HPP


#include <dcs/eesim/performance_measure_category.hpp>
#include <iosfwd>
#include <stdexcept>


namespace dcs { namespace eesim { namespace config {

enum metric_category
{
	busy_time_metric,
	queue_length_metric,
	response_time_metric,
	throughput_metric,
	utilization_metric
};

::dcs::eesim::performance_measure_category to_performance_measure_category(metric_category category)
{
	switch (category)
	{
		case busy_time_metric:
			return ::dcs::eesim::busy_time_performance_measure;
		case queue_length_metric:
			return ::dcs::eesim::queue_length_performance_measure;
		case response_time_metric:
			return ::dcs::eesim::response_time_performance_measure;
		case throughput_metric:
			return ::dcs::eesim::throughput_performance_measure;
		case utilization_metric:
			return ::dcs::eesim::utilization_performance_measure;
	}

	throw ::std::logic_error("[dcs::eesim::config::to_performance_measure_category] Unknown metric category.");
}


metric_category to_metric_category(::dcs::eesim::performance_measure_category category)
{
	switch (category)
	{
		case ::dcs::eesim::busy_time_performance_measure:
			return busy_time_metric;
		case ::dcs::eesim::queue_length_performance_measure:
			return queue_length_metric;
		case ::dcs::eesim::response_time_performance_measure:
			return response_time_metric;
		case ::dcs::eesim::throughput_performance_measure:
			return throughput_metric;
		case ::dcs::eesim::utilization_performance_measure:
			return utilization_metric;
		default:
			throw ::std::logic_error("[dcs::eesim::config::to_metric_category] Unable to convert to performance metric.");
	}
}


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, metric_category category)
{
	switch (category)
	{
		case busy_time_metric:
			os << "busy-time";
			break;
		case queue_length_metric:
			os << "queue-length";
			break;
		case response_time_metric:
			os << "response-time";
			break;
		case throughput_metric:
			os << "throughput";
			break;
		case utilization_metric:
			os << "utilization";
			break;
	}

	return os;
}

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_METRIC_CATEGORY_HPP
