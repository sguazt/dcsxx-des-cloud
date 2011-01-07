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


#include <dcs/assert.hpp>
#include <dcs/eesim/fwd.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/math/stats/distribution/any_distribution.hpp>
#include <dcs/memory.hpp>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>


namespace dcs { namespace eesim {

//template <typename Traits>
//class multi_tier_application;

template <typename TraitsT>
class application_tier
{
	/// The traits type.
	public: typedef TraitsT traits_type;
	/// The type for unsigned integral numbers
	public: typedef typename traits_type::uint_type uint_type;
	/// The type for real numbers
	public: typedef typename traits_type::real_type real_type;
	public: typedef multi_tier_application<traits_type> application_type;
	public: typedef application_type* application_pointer;
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
	private: typedef ::std::map<physical_resource_category,real_type> resource_share_impl_container;
	private: typedef ::std::pair<physical_resource_category,real_type> resource_share_type;
	public: typedef ::std::vector<resource_share_type> resource_share_container;
	public: typedef uint_type identifier_type;


	public: application_tier()
	: id_(0),
	  name_("Unnamed Tier")
	{
	}


	public: ::std::string const& name() const
	{
		return name_;
	}


	public: void name(::std::string const& s)
	{
		name_ = s;
	}


	public: void id(identifier_type id)
	{
		id_ = id;
	}


	public: identifier_type id() const
	{
		return id_;
	}


	public: void application(application_pointer const& ptr_app)
	{
		ptr_app_ = ptr_app;
	}


	public: application_type const& application() const
	{
		return *ptr_app_;
	}


	public: application_type& application()
	{
		return *ptr_app_;
	}


	protected: application_pointer application_ptr() const
	{
		return ptr_app_;
	}


	public: template <typename ForwardIterT> /* <category,share> pairs */
		void resource_shares(ForwardIterT first, ForwardIterT last)
	{
		res_shares_ = resource_share_impl_container(first, last);
	}


	public: void resource_share(physical_resource_category category, real_type share)
	{
		res_shares_[category] = share;
	}


	public: real_type resource_share(physical_resource_category category) const
	{
		typedef typename resource_share_impl_container::const_iterator iterator;

		iterator it(res_shares_.find(category));

		// pre: category must already be inserted.
		DCS_ASSERT(
			it != res_shares_.end(),
			throw ::std::invalid_argument("[dcs::eesim::application_tier::resource_share] Unhandled resource category.")
		);

		return it->second;
	}


	public: resource_share_container resource_shares() const
	{
		return resource_share_container(res_shares_.begin(), res_shares_.end());
	}


//	public: void virtual_machine(virtual_machine_pointer const& ptr_vm)
//	{
//		ptr_vm_ = ptr_vm;
//	}


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


	private: identifier_type id_;
	private: ::std::string name_;
//	private: des_event_source_pointer ptr_arrival_evt_src_;
//	private: des_event_source_pointer ptr_departure_evt_src_;
//	private: des_event_source_pointer ptr_discard_evt_src_;
	private: random_distribution_type svc_dist_;
	private: resource_share_impl_container res_shares_;
	private: application_pointer ptr_app_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_APPLICATION_TIER_HPP
