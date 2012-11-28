/**
 * \file dcs/des/cloud/resource_utilization_profile.hpp
 *
 * \brief Class for resource utilization profiles.
 *
 * Copyright (C) 2009-2012  Distributed Computing System (DCS) Group, Computer
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

#ifndef DCS_DES_CLOUD_RESOURCE_UTILIZATION_PROFILE_HPP
#define DCS_DES_CLOUD_RESOURCE_UTILIZATION_PROFILE_HPP


#include <boost/icl/interval.hpp>
#include <boost/icl/interval_map.hpp>
#include <iosfwd>
#include <utility>


namespace dcs { namespace des { namespace cloud {

//template <typename TraitsT>
//class resource_utilization_profile_item
//{
	//public: typedef TraitsT traits_type;
	//public: typedef typename traits_type::real_type real_type;


	//public: resource_utilization_profile_item()
	//: t1_(0),
	  //t2_(0),
	  //u_(0)
	//{
	//}


	//public: resource_utilization_profile_item(real_type t1, real_type t2, real_type u)
	//: t1_(t1),
	  //t2_(t2),
	  //u_(u)
	//{
	//}


	//public: void begin_time(real_type t)
	//{
		//t1_ = t;
	//}


	//public: real_type begin_time() const
	//{
		//return t1_;
	//}


	//public: void end_time(real_type t)
	//{
		//t2_ = t;	
	//}


	//public: real_type end_time() const
	//{
		//return t2_;
	//}


	//public: void utilization(real_type u)
	//{
		//u_ = u;
	//}


	//public: real_type utilization() const
	//{
		//return u_;
	//}


	//private: real_type t1_;
	//private: real_type t2_;
	//private: real_type u_;
//};


template <typename TraitsT>
class resource_utilization_profile
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
//	public: typedef resource_utilization_profile_item<traits_type> item_type;
	private: typedef ::boost::icl::interval<real_type> interval_type;
	private: typedef ::boost::icl::interval_map<real_type,real_type> interval_container;
	public: typedef typename interval_container::iterator iterator;
	public: typedef typename interval_container::const_iterator const_iterator;
	public: typedef typename interval_container::reverse_iterator reverse_iterator;
	public: typedef typename interval_container::const_reverse_iterator const_reverse_iterator;
	public: typedef typename interval_container::size_type size_type;
	public: typedef typename interval_container::value_type profile_item_type;
	//public: typedef typename interval_container::key_type time_interval_type;
	public: typedef typename interval_type::type time_interval_type;
//	public: typedef typename interval_container::mapped_type profile_item_value_type;


	template <typename TT>
	friend resource_utilization_profile<TT> make_profile_from_intersection(resource_utilization_profile<TT> const& profile, typename resource_utilization_profile<TT>::time_interval_type const& interval);

	template <typename CharT, typename CharTraitsT, typename TT>
	friend ::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, resource_utilization_profile<TT> const& profile);


	/// Default constructor.
	public: resource_utilization_profile()
	{
	}


	/// A constructor.
	protected: explicit resource_utilization_profile(interval_container const& c)
	: profile_(c)
	{
	}


	public: static time_interval_type make_time_interval(real_type t1, real_type t2)
	{
		return interval_type::right_open(t1, t2);
	}


	public: static profile_item_type make_item(real_type t1, real_type t2, real_type u)
	{
		return ::std::make_pair(interval_type::right_open(t1, t2), u);
	}


	public: void operator()(real_type t1, real_type t2, real_type u)
	{
		profile_ += ::std::make_pair(interval_type::right_open(t1, t2), u);
	}


	public: void operator()(profile_item_type const& item)
	{
		profile_ += item;
		//profile_ += ::std::make_pair(interval_type::right_open(::boost::icl::lower(item.first), ::boost::icl::upper(item.first)), item.second);
	}


	public: void operator()(time_interval_type const& interval, real_type u)
	{
		//profile_ += ::std::make_pair(interval_type::right_open(::boost::icl::lower(interval), ::boost::icl::upper(interval)), u);
		profile_ += ::std::make_pair(interval, u);
	}


	public: const_iterator begin() const
	{
		return profile_.begin();
	}


	public: iterator begin()
	{
		return profile_.begin();
	}


	public: const_iterator end() const
	{
		return profile_.end();
	}


	public: iterator end()
	{
		return profile_.end();
	}


	public: const_reverse_iterator rbegin() const
	{
		return profile_.rbegin();
	}


	public: iterator rbegin()
	{
		return profile_.rbegin();
	}


	public: const_reverse_iterator rend() const
	{
		return profile_.rend();
	}


	public: iterator rend()
	{
		return profile_.rend();
	}


	public: size_type size() const
	{
		return profile_.size();
	}


	public: real_type area() const
	{
		real_type a(0);

		const_iterator end_it(end());
		for (const_iterator it = begin(); it != end_it; ++it)
		{
			profile_item_type const& item(*it);

			a += ::boost::icl::length(item.first)*item.second;
		}

		return a;
	}


	private: interval_container profile_;
};


template <typename TraitsT>
typename TraitsT::real_type area(resource_utilization_profile<TraitsT> const& profile)
{
	return profile.area();
}


template <typename TraitsT>
typename TraitsT::real_type area(typename resource_utilization_profile<TraitsT>::profile_item_type const& item)
{
	return static_cast<typename TraitsT::real_type>(::boost::icl::length(item.first))*item.second;
}


template <typename TraitsT>
typename resource_utilization_profile<TraitsT>::difference_type interval_length(typename resource_utilization_profile<TraitsT>::profile_item_type const& item)
{
	return ::boost::icl::length(item.first);
}


template <typename TraitsT>
resource_utilization_profile<TraitsT> make_profile_from_intersection(resource_utilization_profile<TraitsT> const& profile, typename resource_utilization_profile<TraitsT>::time_interval_type const& interval)
{
	return resource_utilization_profile<TraitsT>(profile.profile_ & interval);
}


template <typename CharT, typename CharTraitsT, typename TraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os,
													resource_utilization_profile<TraitsT> const& profile)
{
	os << profile.profile_;

	return os;
}

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_RESOURCE_UTILIZATION_PROFILE_HPP
