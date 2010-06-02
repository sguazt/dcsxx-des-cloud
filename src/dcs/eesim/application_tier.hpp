/**
 * \file dcs/eesim/application_tier.hpp
 *
 * \brief Model for a tier of a multi-tier application.
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
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
#ifndef DCS_EESIM_APPLICATION_TIER_HPP
#define DCS_EESIM_APPLICATION_TIER_HPP


#include <dcs/math/stats/distribution/any_distribution.hpp>
//#include <dcs/memory.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT>
class application_tier
{
	/// The traits type.
	public: typedef TraitsT traits_type;
	/// The type for real numbers
	public: typedef typename traits_type::real_type real_type;
	/// The discrete-event simulation engine type.
//	public: typedef typename traits_type::des_engine_type des_engine_type;
//	/// The type of the events fired by the simulation engine.
//	public: typedef typename des_engine_type::event_type des_event_type;
//	/// The type of the context passed to events fired by the simulation engine.
//	public: typedef typename des_engine_type::engine_context_type des_engine_context_type;
//	/// The type of the source of the events that the simulation engine uses for
//	/// firing events.
//	public: typedef typename des_event_type::event_source_type des_event_source_type;
//	public: typedef ::dcs::shared_ptr<des_event_source_type> des_event_source_pointer;
	public: typedef ::dcs::math::stats::any_distribution<real_type> random_distribution_type;


	public: template <typename RandomNumberDistributionT>
		void service_distribution(RandomNumberDistributionT const& dist)
	{
		svc_dist_ = ::dcs::math::stats::make_any_distribution(dist);
	}


	public: random_distribution_type& service_distribution()
	{
		return svc_dist_;
	}


	public: random_distribution_type const& service_distribution() const
	{
		return svc_dist_;
	}


//	public: void arrival_event_source(des_event_source_pointer const& ptr_event_source)
//	{
//		ptr_arrival_evt_src_ = ptr_event_source;
//	}


//	public: des_event_source_type& arrival_event_source()
//	{
//		return *ptr_arrival_evt_src_;
//	}


//	public: des_event_source_type const& arrival_event_source() const
//	{
//		return *ptr_arrival_evt_src_;
//	}


//	public: void departure_event_source(des_event_source_pointer const& ptr_event_source)
//	{
//		ptr_departure_evt_src_ = ptr_event_source;
//	}


//	public: des_event_source_type& departure_event_source()
//	{
//		return *ptr_departure_evt_src_;
//	}


//	public: des_event_source_type const& departure_event_source() const
//	{
//		return *ptr_departure_evt_src_;
//	}


//	public: void discard_event_source(des_event_source_pointer const& ptr_event_source)
//	{
//		ptr_discard_evt_src_ = ptr_event_source;
//	}


//	public: des_event_source_type& discard_event_source()
//	{
//		return *ptr_discard_evt_src_;
//	}


//	public: des_event_source_type const& discard_event_source() const
//	{
//		return *ptr_discard_evt_src_;
//	}


//	private: des_event_source_pointer ptr_arrival_evt_src_;
//	private: des_event_source_pointer ptr_departure_evt_src_;
//	private: des_event_source_pointer ptr_discard_evt_src_;
	private: random_distribution_type svc_dist_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_APPLICATION_TIER_HPP
