#ifndef DCS_EESIM_BASE_PHYSICAL_MACHINE_CONTROLLER_HPP
#define DCS_EESIM_BASE_PHYSICAL_MACHINE_CONTROLLER_HPP


#include <dcs/eesim/physical_machine.hpp>
#include <dcs/memory.hpp>
#include <limits>


namespace dcs { namespace eesim {

template <typename TraitsT>
class base_physical_machine_controller
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef physical_machine<traits_type> physical_machine_type;
	//public: typedef physical_machine_type* physical_machine_pointer;
	public: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;


	protected: base_physical_machine_controller()
	: ptr_mach_(),
	  ts_(::std::numeric_limits<real_type>::infinity())
	{
	}


	protected: explicit base_physical_machine_controller(physical_machine_pointer const& ptr_mach, real_type ts)
	: ptr_mach_(ptr_mach),
	  ts_(ts)
	{
	}


	public: void controlled_machine(physical_machine_pointer const& ptr_machine)
	{
		ptr_mach_ = ptr_machine;
	}


	public: physical_machine_type& controlledl_machine()
	{
		return *ptr_mach_;
	}


	public: physical_machine_type const& controlledl_machine() const
	{
		return *ptr_mach_;
	}


	public: virtual ~base_physical_machine_controller()
	{
	}


	protected: physical_machine_pointer controlledl_machine_ptr() const
	{
		return ptr_mach_;
	}


	protected: physical_machine_pointer controlledl_machine_ptr()
	{
		return ptr_mach_;
	}


	public: void sampling_time(real_type ts)
	{
		ts_ = ts;
	}


	public: real_type sampling_time() const
	{
		return ts_;
	}


	private: physical_machine_pointer ptr_mach_;
	private: real_type ts_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_PHYSICAL_MACHINE_CONTROLLER_HPP
