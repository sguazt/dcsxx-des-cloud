/**
 * \file dcs/eesim/performance_measure.hpp
 *
 * \brief Class that reprsents a performance measure.
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

#ifndef DCS_EESIM_PERFORMANCE_MEASURE_HPP
#define DCS_EESIM_PERFORMANCE_MEASURE_HPP


#include <dcs/eesim/performance_measure_category.hpp>
#include <limits>


namespace dcs { namespace eesim {

template <typename TraitsT>
class performance_measure
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;


	public: performance_measure()
//	: category_(unknown_performance_measure),
	: category_(),
	  value_(::std::numeric_limits<real_type>::quiet_NaN())
	{
	}

	public: performance_measure(performance_measure_category category, real_type value)
	: category_(category),
	  value_(value)
	{
	}


	public: void category(performance_measure_category x)
	{
		category_ = x;
	}


	public: performance_measure_category category() const
	{
		return category_;
	}


	public: void value(real_type x)
	{
		value_ = x;
	}


	public: real_type value() const
	{
		return value_;
	}


	private: performance_measure_category category_;
	private: real_type value_;
};

}} // Namespace dcs::eesim

#endif // DCS_EESIM_PERFORMANCE_MEASURE_HPP
