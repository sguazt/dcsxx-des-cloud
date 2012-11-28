/**
 * \file dcs/des/cloud/fixed_application_performance_model.hpp
 *
 * \brief Fixed application performance model.
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

#ifndef DCS_DES_CLOUD_FIXED_APPLICATION_PERFORMANCE_MODEL_HPP
#define DCS_DES_CLOUD_FIXED_APPLICATION_PERFORMANCE_MODEL_HPP


#include <dcs/assert.hpp>
#include <dcs/des/cloud/base_application_performance_model.hpp>
#include <dcs/des/cloud/performance_measure_category.hpp>
#include <map>
#include <stdexcept>


namespace dcs { namespace des { namespace cloud {

template <typename TraitsT>
class fixed_application_performance_model: public base_application_performance_model<TraitsT>
{
	private: typedef base_application_performance_model<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	private: typedef ::std::map<performance_measure_category,real_type> measure_map;
	private: typedef ::std::map<uint_type,measure_map> tier_measure_map;


	public: void application_measure(performance_measure_category category, real_type value)
	{
		app_measure_map_[category] = value;
	}


	public: void tier_measure(uint_type tier_id, performance_measure_category category, real_type value)
	{
		tier_measure_map_[tier_id][category] = value;
	}


	private: real_type do_application_measure(performance_measure_category category) const
	{
		// pre: category must already be present
		DCS_ASSERT(
				app_measure_map_.count(category),
				throw ::std::invalid_argument("[dcs::des::cloud::fixed_application_performance_model::do_application_measure] Category not found.")
			);

		return app_measure_map_.at(category);
	}


	private: real_type do_tier_measure(uint_type tier_id, performance_measure_category category) const
	{
		// pre: tier-id must already be present
		DCS_ASSERT(
				tier_measure_map_.count(tier_id),
				throw ::std::invalid_argument("[dcs::des::cloud::fixed_application_performance_model::do_tier_measure] Tier not found.")
			);
		// pre: category must already be present
		DCS_ASSERT(
				tier_measure_map_.at(tier_id).count(category),
				throw ::std::invalid_argument("[dcs::des::cloud::fixed_application_performance_model::do_tier_measure] Category not found.")
			);

		return tier_measure_map_.at(tier_id).at(category);
	}


	private: measure_map app_measure_map_;
	private: tier_measure_map tier_measure_map_;
};

}}} // Namespace dcs::des::cloud


#endif // DCS_FIXED_DES_CLOUD_APPLICATION_PERFORMANCE_MODEL_HPP
