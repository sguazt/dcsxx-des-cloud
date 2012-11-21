/**
 * \file dcs/eesim/user_request.hpp
 *
 * \brief Class modeling a user request.
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

#ifndef DCS_EESIM_USER_REQUEST_HPP
#define DCS_EESIM_USER_REQUEST_HPP


#include <cstddef>
//#include <limits>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/resource_utilization_profile.hpp>
#include <iosfwd>
#include <map>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class user_request
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef uint_type identifier_type;
//	public: typedef resource_utilization_profile_item<traits_type> utilization_profile_item_type;
	public: typedef resource_utilization_profile<traits_type> utilization_profile_type;
	public: typedef typename utilization_profile_type::profile_item_type utilization_profile_item_type;


	private: static const real_type bad_time_;


	public: user_request()
	: arr_time_(0),
	  dep_time_(0)
	{
	}


	public: void id(identifier_type value)
	{
		id_ = value;
	}


	public: identifier_type id() const
	{
		return id_;
	}


	public: void current_tier(uint_type value)
	{
		cur_tier_ = value;
	}


	public: uint_type current_tier() const
	{
		return cur_tier_;
	}


	public: void arrival_time(real_type time)
	{
		arr_time_ = time;
	}


	public: real_type arrival_time() const
	{
		return arr_time_;
	}


	public: void departure_time(real_type time)
	{
		dep_time_ = time;
	}


	public: real_type departure_time() const
	{
		return dep_time_;
	}


	public: void tier_arrival_time(uint_type tier_id, real_type time)
	{
//		if (tier_arr_times_.size() <= tier_id)
//		{
//			tier_arr_times_.resize(tier_id+1, bad_time_);
//		}

		DCS_DEBUG_ASSERT(
					(tier_arr_times_.count(tier_id) == 0 && tier_dep_times_.count(tier_id) == 0)
				||   tier_arr_times_[tier_id].size() == tier_dep_times_[tier_id].size()
			);

		if (tier_arr_times_.count(tier_id) == 0)
		{
			tier_arr_times_[tier_id] = ::std::vector<real_type>();
			tier_dep_times_[tier_id] = ::std::vector<real_type>();
		}

		tier_arr_times_[tier_id].push_back(time);
	}


	public: ::std::vector<real_type> tier_arrival_times(uint_type tier_id) const
	{
//		if (tier_arr_times_.size() <= tier_id)
//		{
////			return bad_time_;
//			return ::std::vector<real_type>();
//		}
		if (tier_arr_times_.count(tier_id) == 0)
		{
			return ::std::vector<real_type>();
		}

		return tier_arr_times_.at(tier_id);
	}


	public: void tier_departure_time(uint_type tier_id, real_type time)
	{
//		if (tier_dep_times_.size() <= tier_id)
//		{
//			tier_dep_times_.resize(tier_id+1, bad_time_);
//		}

		DCS_DEBUG_ASSERT(
					(tier_arr_times_.count(tier_id) == 1 && tier_dep_times_.count(tier_id) == 0)
				||   tier_arr_times_[tier_id].size() == (tier_dep_times_[tier_id].size()+1)
			);

		if (tier_dep_times_.count(tier_id) == 0)
		{
			tier_dep_times_[tier_id] = ::std::vector<real_type>();
		}

		tier_dep_times_[tier_id].push_back(time);
	}


	public: ::std::vector<real_type> tier_departure_times(uint_type tier_id) const
	{
//		if (tier_dep_times_.size() <= tier_id)
//		{
////			return bad_time_;
//			return ::std::vector<real_type>();
//		}
		if (tier_dep_times_.count(tier_id) == 0)
		{
			return ::std::vector<real_type>();
		}

		return tier_dep_times_.at(tier_id);
	}


	public: void tier_utilization_profile(uint_type tier_id, physical_resource_category resource, utilization_profile_type const& profile)
	{
		tier_u_profs_[tier_id][resource].push_back(profile);
	}


	public: ::std::vector<utilization_profile_type> tier_utilization_profiles(uint_type tier_id, physical_resource_category resource) const
	{
		typedef ::std::vector<utilization_profile_type> return_type;

		if (tier_u_profs_.count(tier_id) == 0
			|| tier_u_profs_.at(tier_id).count(resource) == 0)
		{
			return return_type();
		}

		return tier_u_profs_.at(tier_id).at(resource);
	}


	/// The request identifier.
	private: identifier_type id_;
	/// The identifier of tier which is currently serving the request.
	private: uint_type cur_tier_;
	/// The arrival time of the request to the system.
	private: real_type arr_time_;
	/// The departure time of the request from the system.
	private: real_type dep_time_;
	/// The per-tier request arrival times.
	private: ::std::map< uint_type, ::std::vector<real_type> > tier_arr_times_;
	/// The per-tier request departure times.
	private: ::std::map< uint_type, ::std::vector<real_type> > tier_dep_times_;
	/// The per-tier and per-resource utilization profiles
	private: ::std::map< uint_type, ::std::map< physical_resource_category, ::std::vector<utilization_profile_type> > > tier_u_profs_;
};


//template <typename TraitsT>
//const typename user_request<TraitsT>::real_type user_request<TraitsT>::bad_time_ = ::std::numeric_limits<typename user_request<TraitsT>::real_type>::infinity();

template <
    typename CharT,
    typename CharTraitsT,
    typename TraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, user_request<TraitsT> const& req)
{
    return os << "<"
			  <<   "ID: " << req.id()
			  << ", Tier: " << req.current_tier()
			  << ", Arrival Time: " << req.arrival_time()
			  << ", Departure Time: " << req.departure_time()
			  << ">";
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_USER_REQUEST_HPP
