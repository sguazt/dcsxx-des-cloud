#ifndef DCS_EESIM_USER_REQUEST_HPP
#define DCS_EESIM_USER_REQUEST_HPP


namespace dcs { namespace eesim {


template <typename RealT>
class user_request
{
	public: typedef RealT real_type;


	public: user_request()
	: arr_time_(0),
	  dep_time_(0)
	{
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


	private: real_type arr_time_;
	private: real_type dep_time_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_USER_REQUEST_HPP
