/**
 * \file dcs/eesim/application_simulation_model_traits.hpp
 *
 * \brief Traits class for application simulation model.
 *
 * Copyright (C) 2009-2011  Distributed Computing System (DCS) Group, Computer
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

#ifndef DCS_EESIM_APPLICATION_SIMULATION_MODEL_TRAITS_HPP
#define DCS_EESIM_APPLICATION_SIMULATION_MODEL_TRAITS_HPP


#include <boost/any.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT, typename ModelT>
class application_simulation_model_traits
{
	public: typedef TraitsT traits_type;
	public: typedef ModelT model_type;
	private: typedef typename traits_type::engine_type des_engine_type;
	public: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	public: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename traits_type::uint_type real_type;
	public: typedef ::dcs::des::base_statistic<real_type,uint_type> output_statistic_type;
	public: typedef ::dcs::shared_ptr<output_statistic_type> output_statistic_pointer;
	public: typedef ::boost::any foreign_identifier_type;
	public: typedef typename traits_type::user_request_type user_request_type;


	public: static void enable(model_type& model, bool flag);


	public: static bool enabled(model_type const& model);


	public: static des_event_source_type& request_arrival_event_source(model_type& model);

	public: static des_event_source_type const& request_arrival_event_source(model_type const& model);

	public: static des_event_source_type& request_departure_event_source(model_type& model);

	public: static des_event_source_type const& request_departure_event_source(model_type const& model);


	public: static des_event_source_type& request_tier_arrival_event_source(model_type& model, uint_type tier_id);

	public: static des_event_source_type const& request_tier_arrival_event_source(model_type const& model, uint_type tier_id);

	public: static des_event_source_type& request_tier_departure_event_source(model_type& model, uint_type tier_id);

	public: static des_event_source_type const& request_tier_departure_event_source(model_type const& model, uint_type tier_id);

    public: static void statistic(model_type& model, performance_measure_category category, output_statistic_pointer const& ptr_stat);

    public: static output_statistic_pointer statistic(model_type const& model, performance_measure_category category);

    public: static void tier_statistic(model_type& model, performance_measure_category category, foreign_identifier_type tier_id, output_statistic_pointer const& ptr_stat);

    public: static output_statistic_pointer tier_statistic(model_type const& model, foreign_identifier_type tier_id, performance_measure_category category);


	public: static bool has_performance_measure(model_type const& model, performance_measure_category category);


	public: static user_request_type request_state(model_type const& model, des_event_type const& evt);


	public: static uint_type actual_num_arrivals(model_type const& model);


	public: static uint_type actual_num_departures(model_type const& model);


//	public: static real_type actual_busy_time(model_type const& model);


	public: static uint_type actual_tier_num_arrivals(model_type const& model, foreign_identifier_type foreign_id);


	public: static uint_type actual_tier_num_departures(model_type const& model, foreign_identifier_type foreign_id);


	public: static real_type actual_tier_busy_time(model_type const& model, foreign_identifier_type foreign_id);
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_APPLICATION_SIMULATION_MODEL_TRAITS_HPP
