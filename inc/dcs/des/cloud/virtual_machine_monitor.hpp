/**
 * \file dcs/eesim/virtual_machine_monitor.hpp
 *
 * \brief Virtual machine monitor.
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

#ifndef DCS_EESIM_VIRTUAL_MACHINE_MONITOR_HPP
#define DCS_EESIM_VIRTUAL_MACHINE_MONITOR_HPP


#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/power_status.hpp>
#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/exception.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
//#include <map>
#include <stdexcept>
#include <map>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class physical_machine;

template <typename TraitsT>
class virtual_machine_monitor//: public base_virtual_machine_monitor<TraitsT>
{
	//public: typedef any_queue_model queue_model_type;
	//public: typedef virtual_machine<queue_model_type> virtual_machine_type;
	//private: typedef base_virtual_machine_monitor<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
//	public: typedef physical_machine<traits_type> physical_machine_type;
//	public: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	public: typedef virtual_machine<traits_type> virtual_machine_type;
	public: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	public: typedef physical_machine<traits_type> physical_machine_type;
	public: typedef physical_machine_type* physical_machine_pointer;
	//private: typedef ::std::map<uint_type,virtual_machine_pointer> virtual_machine_container;
	public: typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;
	private: typedef ::std::map<virtual_machine_identifier_type,virtual_machine_pointer> virtual_machine_impl_container;


	public: virtual_machine_monitor()
	: ptr_pm_(0)
//		: vms_counter_(0)
	{
	}


	/// Copy constructor.
	private: virtual_machine_monitor(virtual_machine_monitor const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: virtual_machine_monitor& operator=(virtual_machine_monitor const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
	}


//	public: template <typename ForwardIteratorT>
//		virtual_machine_identifier create_virtual_machine(application_pointer const& ptr_app, ForwardIteratorT resource_shares_first, ForwardIteratorT resource_shares_last)
//	{
//		//++vms_counter_;
//
//		vms_.push_back(ptr_vm);
//		//vms_.insert(::std::make_pair(vms_counter_, ptr_vm));
//		//vm_id_names.insert(::std::make_pair(vms_counter_, ptr_vm->name()));
//
//		//return vms_counter_;
//		return (vms_.size()-1);
//	}


//	public: physical_machine_pointer hosting_machine() const
//	{
//		return ptr_pm_;
//	}


//	public: physical_machine_pointer hosting_machine()
//	{
//		return ptr_pm_;
//	}


	public: void create_domain(virtual_machine_pointer const& ptr_vm)
	{
		DCS_ASSERT(
			ptr_vm,
			throw ::std::invalid_argument("[dcs::eesim::virtual_machine_monitor::create_domain] Invalid virtual machine pointer.")
		);

		ptr_vm->vmm(this);
		vms_[ptr_vm->id()] = ptr_vm;
	}


	public: void destroy_domain(virtual_machine_pointer const& ptr_vm)
	{
		DCS_ASSERT(
			ptr_vm,
			throw ::std::invalid_argument("[dcs::eesim::virtual_machine_monitor::destroy_domain] Invalid virtual machine pointer.")
		);
		DCS_ASSERT(
			vms_.count(ptr_vm->id()) > 0,
			throw ::std::runtime_error("[dcs::eesim::virtual_machine_monitor::destroy_domain] Unknown virtual machine.")
		);

		if (ptr_vm->power_state() == powered_on_power_status)
		{
			this->power_off(ptr_vm);
		}

		ptr_vm->vmm(0);
		vms_.erase(ptr_vm->id());
	}


	public: virtual_machine_container virtual_machines() const
	{
		typedef typename virtual_machine_impl_container::const_iterator iterator;

		virtual_machine_container vms;

		iterator end_it = vms_.end();
		for (iterator it = vms_.begin(); it != end_it; ++it)
		{
			virtual_machine_pointer ptr_vm(it->second);

			vms.push_back(ptr_vm);
		}

		return vms;
	}


	public: virtual_machine_container virtual_machines(power_status status) const
	{
		typedef typename virtual_machine_impl_container::const_iterator iterator;

		virtual_machine_container vms;

		iterator end_it = vms_.end();
		for (iterator it = vms_.begin(); it != end_it; ++it)
		{
			virtual_machine_pointer ptr_vm(it->second);

			if (ptr_vm->power_state() == status)
			{
				vms.push_back(ptr_vm);
			}
		}

		return vms;
	}


	public: virtual_machine_pointer virtual_machine_ptr(virtual_machine_identifier_type id) const
	{
		DCS_ASSERT(
				vms_.count(id) > 0,
				throw ::std::runtime_error("[dcs::eesim::virtual_machine_monitor::virtual_machine_ptr] VM not found.")
			);

		return vms_.at(id);
	}


//	public: void overhead(real_type value)
//	{
//		overhead_ = value;
//	}


//	public: real_type overhead() const
//	{
//		return overhead_;
//	}


	public: void power_on(virtual_machine_pointer const& ptr_vm) // throws...
	{
		// preconditions
		DCS_ASSERT(
			vms_.count(ptr_vm->id()) > 0,
			throw ::std::runtime_error("[dcs::eesim::virtual_machine_monitor::power_on] Unknown virtual machine.")
		);
		DCS_DEBUG_ASSERT( ptr_pm_ );

//		ptr_vm->power_on();

		ptr_pm_->simulation_model().vm_power_on(ptr_vm);
	}


	public: void power_off(virtual_machine_pointer const& ptr_vm) // throws...
	{
		// preconditions
		DCS_ASSERT(
			vms_.count(ptr_vm->id()) > 0,
			throw ::std::runtime_error("[dcs::eesim::virtual_machine_monitor::power_off] Unknown virtual machine.")
		);
		DCS_DEBUG_ASSERT( ptr_pm_ );

//		ptr_vm->power_off();

		ptr_pm_->simulation_model().vm_power_off(ptr_vm);
	}


	public: void suspend(virtual_machine_pointer const& ptr_vm) // throws...
	{
		// preconditions
		DCS_ASSERT(
			vms_.count(ptr_vm->id()) > 0,
			throw ::std::runtime_error("[dcs::eesim::virtual_machine_monitor::suspend] Unknown virtual machine.")
		);
		DCS_DEBUG_ASSERT( ptr_pm_ );

		ptr_pm_->simulation_model().vm_suspend(ptr_vm);
	}


	public: void resume(virtual_machine_pointer const& ptr_vm) // throws...
	{
		// preconditions
		DCS_ASSERT(
			vms_.count(ptr_vm->id()) > 0,
			throw ::std::runtime_error("[dcs::eesim::virtual_machine_monitor::resume] Unknown virtual machine.")
		);
		DCS_DEBUG_ASSERT( ptr_pm_ );

		ptr_pm_->simulation_model().vm_resume(ptr_vm);
	}


	public: void migrate(virtual_machine_pointer ptr_vm,
						 physical_machine_type& pm)
	{
		// pre: target virtual machine must be set.
		DCS_ASSERT(
			ptr_vm,
			throw ::std::logic_error("[dcs::eesim::virtual_machine_monitor::migrate] target Virtual Machine not set.")
		);
		// pre: target physical machine must be set.
		DCS_ASSERT(
			pm.power_state() == powered_on_power_status,
			throw ::std::logic_error("[dcs::eesim::virtual_machine_monitor::migrate] target Physical Machine not powered on.")
		);

		ptr_pm_->simulation_model().vm_migrate(ptr_vm, pm, false);
		pm.simulation_model().vm_migrate(ptr_vm, *ptr_pm_, true);
		pm.vmm().create_domain(ptr_vm);
		destroy_domain(ptr_vm);
	}


	public: template <typename ForwardIterT>
		void migrate(virtual_machine_pointer ptr_vm,
					 physical_machine_type& pm,
					 ForwardIterT first_share,
					 ForwardIterT last_share)
	{
		migrate(ptr_vm, pm);
		ptr_vm->resource_shares(first_share, last_share);
	}


	public: void hosting_machine(physical_machine_pointer const& ptr_mach)
	{
		ptr_pm_ = ptr_mach;
	}


	public: physical_machine_type const& hosting_machine() const
	{
		// pre: hosting machine must already be set.
		DCS_ASSERT(
			ptr_pm_,
			throw ::std::logic_error("[dcs::eesim::virtual_machine_monitor::hosting_machine] Physical Machine not set.")
		);

		return *ptr_pm_;
	}


	public: physical_machine_type& hosting_machine()
	{
		// pre: hosting machine must already be set.
		DCS_ASSERT(
			ptr_pm_,
			throw ::std::logic_error("[dcs::eesim::virtual_machine_monitor::hosting_machine] Physical Machine not set.")
		);

		return *ptr_pm_;
	}


//	private: uint_type vms_counter_;
	private: virtual_machine_impl_container vms_;
	//private: uint_type_name_container vm_id_names_;
	/// The performance overhead associated to virtualization
	private: physical_machine_pointer ptr_pm_;
	private: real_type overhead_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_VIRTUAL_MACHINE_MONITOR_HPP
