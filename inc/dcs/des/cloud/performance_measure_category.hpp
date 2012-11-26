/**
 * \file dcs/eesim/performance_measure_category.hpp
 *
 * \brief Categories for application statistics.
 *
 * Copyright (C) 2009-2010  Distributed Computing System (DCS) Group, Computer
 * Science Department - University of Piemonte Orientale, Alessandria (Italy).
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
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_EESIM_PERFORMANCE_MEASURE_CATEGORY_HPP
#define DCS_EESIM_PERFORMANCE_MEASURE_CATEGORY_HPP


#include <dcs/macro.hpp>
#include <vector>


namespace dcs { namespace eesim {

enum performance_measure_category
{
	busy_time_performance_measure,
	queue_length_performance_measure,
	response_time_performance_measure,
	throughput_performance_measure,
	utilization_performance_measure
//	unknown_performance_measure*/
};


inline
::std::vector<performance_measure_category> performance_measure_categories()
{
	::std::vector<performance_measure_category> cats;

	cats.push_back(busy_time_performance_measure);
	cats.push_back(queue_length_performance_measure);
	cats.push_back(response_time_performance_measure);
	cats.push_back(throughput_performance_measure);
	cats.push_back(utilization_performance_measure);

	return cats;
}


inline
bool for_application(performance_measure_category category)
{
	return category != busy_time_performance_measure
		   && category != queue_length_performance_measure
		   && category != utilization_performance_measure;
}


inline
bool for_application_tier(performance_measure_category category)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( category );

	return true;
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_PERFORMANCE_MEASURE_CATEGORY_HPP
