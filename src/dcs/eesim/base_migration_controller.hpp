#ifndef DCS_EESIM_BASE_MIGRATION_CONTROLLER_HPP
#define DCS_EESIM_BASE_MIGRATION_CONTROLLER_HPP


#include <dcs/eesim/data_center.hpp>
#include <dcs/memory.hpp>
#include <limits>


namespace dcs { namespace eesim {

template <typename TraitsT>
class base_migration_controller
{
//TODO

	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef data_center<traits_type> data_center_type;
	public: typedef ::dcs::shared_ptr<data_center_type> data_center_pointer;


	protected: base_migration_controller()
	: ptr_dc_(),
	  ts_(::std::numeric_limits<real_type>::infinity())
	{
	}


	public: virtual ~base_migration_controller()
	{
	}


	public: void controlled_data_center(data_center_pointer const& ptr_dc)
	{
		ptr_dc_ = ptr_dc;
	}


	public: data_center_pointer const& controlled_data_center()
	{
		return ptr_dc_;
	}


	public: data_center_pointer const& controlled_data_center() const
	{
		return ptr_dc_;
	}


	public: void sampling_time(real_type ts)
	{
		ts_ = ts;
	}


	public: real_type sampling_time() const
	{
		return ts_;
	}


	private: data_center_pointer ptr_dc_;
	private: real_type ts_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_MIGRATION_CONTROLLER_HPP
