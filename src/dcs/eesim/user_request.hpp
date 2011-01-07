#ifndef DCS_EESIM_USER_REQUEST_HPP
#define DCS_EESIM_USER_REQUEST_HPP


#include <cstddef>
//#include <limits>
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
};


//template <typename TraitsT>
//const typename user_request<TraitsT>::real_type user_request<TraitsT>::bad_time_ = ::std::numeric_limits<typename user_request<TraitsT>::real_type>::infinity();

}} // Namespace dcs::eesim


#endif // DCS_EESIM_USER_REQUEST_HPP
