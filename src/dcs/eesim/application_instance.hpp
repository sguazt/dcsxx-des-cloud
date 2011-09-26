#ifndef DCS_EESIM_APPLICATION_INSTANCE_HPP
#define DCS_EESIM_APPLICATION_INSTANCE_HPP


#include <dcs/eesim/base_application_controller.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT>
class application_instance
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef multi_tier_application<traits_type> application_type;
	public: typedef ::dcs::shared_ptr<application_type> application_pointer;
	public: typedef base_application_controller<traits_type> application_controller_type;
	public: typedef ::dcs::shared_ptr<application_controller_type> application_controller_pointer;


	public: application_instance(application_pointer const& ptr_app,
								 application_controller_pointer const& ptr_app_ctrl,
								 real_type start_time,
								 real_type run_time)
	: ptr_app_(ptr_app),
	  ptr_app_ctrl_(ptr_app_ctrl),
	  start_time_(start_time),
	  stop_time_(start_time+run_time)
	{
	}


	public: application_pointer application_ptr() const
	{
		return ptr_app_;
	}


	public: application_pointer application_ptr()
	{
		return ptr_app_;
	}


	public: application_type const& application() const
	{
		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_app_ );

		return ptr_app_;
	}


	public: application_type& application()
	{
		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_app_ );

		return ptr_app_;
	}


	public: application_controller_pointer application_controller_ptr() const
	{
		return ptr_app_ctrl_;
	}


	public: application_controller_pointer application_controller_ptr()
	{
		return ptr_app_ctrl_;
	}


	public: application_controller_type const& application_controller() const
	{
		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_app_ctrl_ );

		return ptr_app_ctrl_;
	}


	public: application_controller_type& application_controller()
	{
		// paranoid-check: valid pointer
		DCS_DEBUG_ASSERT( ptr_app_ctrl_ );

		return ptr_app_ctrl_;
	}


	public: real_type start_time() const
	{
		return start_time_;
	}


	public: real_type stop_time() const
	{
		return stop_time_;
	}


	protected: void application(application_pointer const& ptr_app)
	{
		ptr_app_ = ptr_app;
	}


	protected: void application_controller(application_controller_pointer const& ptr_app_ctrl)
	{
		ptr_app_ctrl_ = ptr_app_ctrl;
	}


	protected: void start_time(real_type time)
	{
		start_time_ = time;
	}


	protected: void stop_time(real_type time)
	{
		stop_time_ = time;
	}


	private: application_pointer ptr_app_;
	private: application_controller_pointer ptr_app_ctrl_;
	private: real_type start_time_;
	private: real_type stop_time_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_APPLICATION_INSTANCE_HPP
