/**
 * \file dcs/eesim/virtual_machine.hpp
 *
 * \brief Model for virtual machines.
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

#ifndef DCS_EESIM_VIRTUAL_MACHINE_HPP
#define DCS_EESIM_VIRTUAL_MACHINE_HPP


#include <algorithm>
#include <dcs/debug.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <dcs/eesim/application_tier.hpp>
#include <dcs/eesim/fwd.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/power_status.hpp>
//#include <dcs/eesim/virtual_machine_monitor.hpp>
#include <dcs/memory.hpp>
#include <map>
#include <string>
#include <utility>
#include <vector>


namespace dcs { namespace eesim {

//template <typename TraitsT>
//class physical_machine;
//template <typename TraitsT>
//class virtual_machine_monitor;

/**
 * \brief Model for virtual machines.
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename TraitsT>
class virtual_machine
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::virtual_machine_identifier_type identifier_type;
	private: typedef ::std::map<physical_resource_category,real_type> resource_share_impl_container;
	private: typedef ::std::pair<physical_resource_category, real_type> resource_share_type;
	public: typedef ::std::vector<resource_share_type> resource_share_container;
	public: typedef application_tier<traits_type> application_tier_type;
	public: typedef ::dcs::shared_ptr<application_tier_type> application_tier_pointer;
	public: typedef physical_machine<traits_type> physical_machine_type;
//	public: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
//	public: typedef ::std::vector< physical_resource<traits_type> > resource_container;
	private: typedef typename application_tier_type::application_type application_type;
	public: typedef ::std::vector<typename application_type::reference_physical_resource> resource_container;
//	public: typedef virtual_machine_monitor<traits_type> virtual_machine_monitor_type;
//	public: typedef ::dcs::shared_ptr<virtual_machine_monitor_type> virtual_machine_monitor_pointer;


	public: explicit virtual_machine(std::string const& name="Unnamed VM")
		: id_(traits_type::invalid_virtual_machine_id),
		  name_(name),
		  power_status_(powered_off_power_status)
	{
	}


//	public: void guest_system(::dcs::shared_ptr<virtual_guest_system> const& ptr_guest)
//	{
//		app_ = make_any_application(ptr_app)
//	}


	public: void id(identifier_type value)
	{
		id_ = value;
	}


	public: identifier_type id() const
	{
		return id_;
	}


//	public: void application(application_pointer const& ptr_app)
//	{
//		ptr_app_ = ptr_app;
//	}


	public: void guest_system(application_tier_pointer const& ptr_tier)
	{
		ptr_tier_ = ptr_tier;
	}


	public: application_tier_type const& guest_system() const
	{
		return *ptr_tier_;
	}


	public: application_tier_type& guest_system()
	{
		return *ptr_tier_;
	}


//	public: void vmm(virtual_machine_monitor_pointer const& ptr_vmm)
//	{
//		ptr_vmm_ = ptr_vmm;
//	}


//	public: virtual_machine_monitor_pointer vmm_ptr() const
//	{
//		return ptr_vmm_;
//	}


//	public: virtual_machine_monitor_pointer vmm_ptr()
//	{
//		return ptr_vmm_;
//	}


//	public: void host_machine(physical_machine_pointer const& ptr_mach)
//	{
//		ptr_mach_ = ptr_mach;
//	}


//	public: physical_machine_pointer host_machine() const
//	{
//		return ptr_mach_;
//	}


//	public: physical_machine_pointer host_machine()
//	{
//		return ptr_mach_;
//	}


	public: resource_container reference_resources() const
	{
		// precondition: tier pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_tier_ );

		return ptr_tier_->application().reference_resources();
	}


//	public: void wanted_resource_share(physical_resource_category category, real_type fraction)
//	{
//		wanted_res_shares_[category] = fraction;
//	}


//	public: template <typename ForwardIteratorT>
//		void wanted_resource_shares(ForwardIteratorT first, ForwardIteratorT last)
//	{
//		wanted_res_shares_ = resource_container(first, last);
//	}


	public: resource_share_container wanted_resource_shares() const
	{
		// precondition: tier pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_tier_ );

		return ptr_tier_->resource_shares();
	}


	public: resource_share_container wanted_resource_shares(physical_machine_type const& mach) const
	{
		// precondition: tier pointer must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_tier_ );

		resource_share_container wanted_shares = ptr_tier_->resource_shares();
		typename resource_share_container::iterator end_it = wanted_shares.end();
		for (typename resource_share_container::iterator it = wanted_shares.begin();
			 it != end_it;
			 ++it)
		{
			physical_resource_category category = it->first;
			real_type ref_capacity = ptr_tier_->application().reference_resource(category).capacity();
			real_type actual_capacity = mach.resource(category)->capacity();
			real_type wanted_share = it->second*ref_capacity/actual_capacity;

//			wanted_shares[category] = wanted_share;
			real_type max_share = mach.resource(category)->utilization_threshold();
			it->second = ::std::min(wanted_share, max_share);
		}

		return wanted_shares;
	}


	/// Set the resource share for the given resource category.
	public: void resource_share(physical_resource_category category, real_type fraction)
	{
		res_shares_[category] = fraction;
	}


	/// Fill the resource shares from pairs of
	/// <resource category,resource fraction>.
	public: template <typename ForwardIteratorT>
		void resource_shares(ForwardIteratorT first, ForwardIteratorT last)
	{
		res_shares_ = resource_share_impl_container(first, last);
	}


	public: resource_share_container resource_shares() const
	{
		return resource_share_container(res_shares_.begin(), res_shares_.end());
	}


	public: void power_on()
	{
DCS_DEBUG_TRACE("Powering-on VM: " << this->id());//XXX
		power_status_ = powered_on_power_status;
	}


	public: void power_off()
	{
DCS_DEBUG_TRACE("Powering-off VM: " << this->id());//XXX
		power_status_ = powered_off_power_status;
	}


	public: void suspend()
	{
		// precondition: power status must be POWERED-ON
		DCS_ASSERT(
			power_status_ == powered_on_power_status,
			::std::logic_error("[dcs::eesim::virtual_machine::suspend] Cannot suspend a non-running virtual machine.")
		);

		power_status_ = suspended_power_status;
	}


	public: void resume()
	{
		// precondition: power status must be SUSPENDED
		DCS_ASSERT(
			power_status_ == suspended_power_status,
			::std::logic_error("[dcs::eesim::virtual_machine::resume] Cannot resume a non-suspended virtual machine.")
		);

		power_status_ = powered_on_power_status;
	}


	public: power_status power_state() const
	{
		return power_status_;
	}


	protected: application_tier_pointer guest_system_ptr() const
	{
		return ptr_tier_;
	}


	protected: application_tier_pointer guest_system_ptr()
	{
		return ptr_tier_;
	}


	private: identifier_type id_;
	private: ::std::string name_;
//	private: queue_model_type queue_;
	private: power_status power_status_;
	private: resource_share_impl_container res_shares_;
	private: application_tier_pointer ptr_tier_;
//	private: physical_machine_pointer ptr_mach_;
//	private: virtual_machine_monitor_pointer ptr_vmm_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_VIRTUAL_MACHINE_HPP
