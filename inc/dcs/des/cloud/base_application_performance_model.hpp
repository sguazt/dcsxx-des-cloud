/**
 * \file dcs/des/cloud/base_application_performance_model.hpp
 *
 * \brief Base class for application performance models.
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

#ifndef DCS_DES_CLOUD_BASE_APPLICATION_PERFORMANCE_MODEL_HPP
#define DCS_DES_CLOUD_BASE_APPLICATION_PERFORMANCE_MODEL_HPP


#include <dcs/des/cloud/performance_measure_category.hpp>


namespace dcs { namespace des { namespace cloud {

template <typename TraitsT>
class base_application_performance_model
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;


	public: real_type application_measure(performance_measure_category category) const
	{
		return do_application_measure(category);
	}


	public: real_type tier_measure(uint_type tier_id, performance_measure_category category) const
	{
		return do_tier_measure(tier_id, category);
	}


	private: virtual real_type do_application_measure(performance_measure_category category) const = 0;

	private: virtual real_type do_tier_measure(uint_type tier_id, performance_measure_category category) const = 0;
};

}}} // Namespace dcs::des::cloud


#endif // DCS_BASE_DES_CLOUD_APPLICATION_PERFORMANCE_MODEL_HPP
