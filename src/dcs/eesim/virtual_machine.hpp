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
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/base_analyzable_statistic.hpp>
#include <dcs/des/max_estimator.hpp>
#include <dcs/des/mean_estimator.hpp>
#include <dcs/des/min_estimator.hpp>
#include <dcs/des/quantile_estimator.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <dcs/eesim/application_tier.hpp>
#include <dcs/eesim/fwd.hpp>
#include <dcs/eesim/logging.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/physical_resource_view.hpp>
#include <dcs/eesim/power_status.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/utility.hpp>
#include <dcs/eesim/virtual_machine_monitor.hpp>
#include <dcs/exception.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <iomanip>
#include <iosfwd>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


namespace dcs { namespace eesim {

//template <typename TraitsT>
//class physical_machine;

template <typename TraitsT>
class virtual_machine_monitor;


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
	public: typedef typename traits_type::uint_type uint_type;
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
	public: typedef physical_resource_view<traits_type> physical_resource_view_type;
	public: typedef ::std::vector<physical_resource_view_type> resource_container;
	public: typedef virtual_machine_monitor<traits_type> virtual_machine_monitor_type;
	public: typedef virtual_machine_monitor_type* virtual_machine_monitor_pointer;
	private: typedef ::dcs::des::base_analyzable_statistic<real_type,uint_type> statistic_type;
	public: typedef ::dcs::shared_ptr<statistic_type> statistic_pointer;
	public: typedef ::std::vector<statistic_pointer> statistic_container;
	private: typedef ::std::map<physical_resource_category,statistic_container> resource_share_stat_impl_container;


	/// Default constructor.
	public: explicit virtual_machine(std::string const& name="Unnamed VM")
		: id_(traits_type::invalid_virtual_machine_id),
		  name_(name),
		  power_status_(powered_off_power_status),
		  ptr_vmm_(0)
	{
		// empty
	}


	/// Copy constructor.
	private: virtual_machine(virtual_machine const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assignment.
	private: virtual_machine& operator=(virtual_machine const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assigment not yet implemented." );
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


	public: void name(::std::string const& name)
	{
		name_ = name;
	}


	public: ::std::string const& name() const
	{
		return name_;
	}


//	public: void application(application_pointer const& ptr_app)
//	{
//		ptr_app_ = ptr_app;
//	}


	public: bool assigned() const
	{
		return ptr_tier_;
	}


	public: bool deployed() const
	{
		return ptr_vmm_;
	}


	public: void guest_system(application_tier_pointer const& ptr_tier)
	{
		ptr_tier_ = ptr_tier;

		if (ptr_tier_)
		{
			resource_container resources(reference_resources());
			typedef typename resource_container::const_iterator resource_iterator;
			resource_iterator res_end_it(resources.end());
			for (resource_iterator it = resources.begin(); it != res_end_it; ++it)
			{
				create_share_stats(it->category());
			}

			resource_share_container wanted_shares(ptr_tier_->resource_shares());
			wanted_resource_shares(wanted_shares.begin(), wanted_shares.end());
		}
		else
		{
			reset_wanted_share_stats();
			reset_share_stats();
		}
	}


	public: application_tier_type const& guest_system() const
	{
		// pre: application tier must already be set
		DCS_ASSERT(
			ptr_tier_,
			throw ::std::logic_error("[dcs::eesim::virtual_machine::guest_system] Guest system has not been set yet.")
		);

		return *ptr_tier_;
	}


	public: application_tier_type& guest_system()
	{
		// pre: application tier must already be set
		DCS_ASSERT(
			ptr_tier_,
			throw ::std::logic_error("[dcs::eesim::virtual_machine::guest_system] Guest system has not been set yet.")
		);

		return *ptr_tier_;
	}


	public: void vmm(virtual_machine_monitor_pointer const& ptr_vmm)
	{
		ptr_vmm_ = ptr_vmm;
	}


	public: virtual_machine_monitor_type& vmm()
	{
		// pre: VMM must already be set
		DCS_ASSERT(
			ptr_vmm_,
			throw ::std::logic_error("[dcs::eesim::virtual_machine::vmm] Virtual Machine Monitor not set.")
		);

		return *ptr_vmm_;
	}


	public: virtual_machine_monitor_type const& vmm() const
	{
		// pre: VMM must already be set
		DCS_ASSERT(
			ptr_vmm_,
			throw ::std::logic_error("[dcs::eesim::virtual_machine::vmm] Virtual Machine Monitor not set.")
		);

		return *ptr_vmm_;
	}


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


	public: void wanted_resource_share(physical_resource_category category, real_type fraction)
	{
		wanted_res_shares_[category] = fraction;

		if (power_status_ == powered_on_power_status)
		{
			update_wanted_share_stats(category, fraction);
		}
	}


	public: template <typename ForwardIteratorT>
		void wanted_resource_shares(ForwardIteratorT first, ForwardIteratorT last)
	{
		wanted_res_shares_ = resource_share_impl_container(first, last);

		if (power_status_ == powered_on_power_status)
		{
			update_wanted_share_stats(first, last);
		}
	}


	public: resource_share_container wanted_resource_shares() const
	{
//		// precondition: tier pointer must be a valid pointer
//		DCS_DEBUG_ASSERT( ptr_tier_ );
//
//		return ptr_tier_->resource_shares();
		return resource_share_container(wanted_res_shares_.begin(),
										wanted_res_shares_.end());
	}


	public: real_type wanted_resource_share(physical_resource_category category) const
	{
		DCS_ASSERT(
			wanted_res_shares_.count(category) > 0,
			throw ::std::invalid_argument("[dcs::eesim::virtual_machine::wanted_resource_share] Unhandled resource category.")
		);

		return wanted_res_shares_.at(category);
	}


	public: ::std::vector<statistic_pointer> wanted_resource_share_statistics(physical_resource_category category) const
	{
		return wanted_res_shares_stats_.at(category);
	}


	public: ::std::map< physical_resource_category, ::std::vector<statistic_pointer> > wanted_resource_share_statistics() const
	{
		return wanted_res_shares_stats_;
	}


	/// Set the resource share for the given resource category.
	public: void resource_share(physical_resource_category category, real_type share)
	{
		res_shares_[category] = share;

		if (this->guest_system_ptr() && this->deployed())
		{
			application_tier_type& tier(this->guest_system());

			tier.application().simulation_model().resource_share(
					tier.id(),
					category,
					share
				);

			if (power_status_ == powered_on_power_status)
			{
				update_share_stats(category, share);
			}
		}
		else
		{
			// Issue a warning. This is not an error since we may call this
			// method when a VM is still to be deployed.
			::std::ostringstream oss;
			oss << "Virtual Machine " << *this << " not correctly deployed.";
			log_warn(DCS_EESIM_LOGGING_AT, oss.str());
		}
	}


	/// Fill the resource shares from pairs of
	/// <resource category,resource fraction>.
	public: template <typename ForwardIteratorT>
		void resource_shares(ForwardIteratorT first, ForwardIteratorT last)
	{
//		res_shares_ = resource_share_impl_container(first, last);
//
//		if (power_status_ == powered_on_power_status)
//		{
//			update_share_stats(first, last);
//		}
		res_shares_.clear();
		while (first != last)
		{
			resource_share(first->first, first->second);
			++first;
		}
	}


	public: resource_share_container resource_shares() const
	{
		return resource_share_container(res_shares_.begin(), res_shares_.end());
	}


	public: real_type resource_share(physical_resource_category category) const
	{
		DCS_ASSERT(
			res_shares_.count(category) > 0,
			throw ::std::invalid_argument("[dcs::eesim::virtual_machine::resource_share] Unhandled resource category.")
		);

		return res_shares_.at(category);
	}


	public: ::std::vector<statistic_pointer> resource_share_statistics(physical_resource_category category) const
	{
		return res_shares_stats_.at(category);
	}


	public: ::std::map< physical_resource_category, ::std::vector<statistic_pointer> > resource_share_statistics() const
	{
		return res_shares_stats_;
	}


	public: void power_on()
	{
		DCS_DEBUG_ASSERT( guest_system_ptr() && deployed() );

		power_status old_status(power_status_);

		power_status_ = powered_on_power_status;

		// Reassign the share to the simulation model
		typedef typename resource_share_impl_container::const_iterator share_iterator;
		application_tier_type& tier(guest_system());
		share_iterator share_end_it(res_shares_.end());
		for (share_iterator it = res_shares_.begin(); it != share_end_it; ++it)
		{
			tier.application().simulation_model().resource_share(
					tier.id(),
					it->first,
					it->second
				);
		}

		if (old_status != power_status_)
		{
			update_wanted_share_stats(wanted_res_shares_.begin(), wanted_res_shares_.end());
			update_share_stats(res_shares_.begin(), res_shares_.end());
		}
	}


	public: void power_off()
	{
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

		update_wanted_share_stats(wanted_res_shares_.begin(), wanted_res_shares_.end());
		update_share_stats(res_shares_.begin(), res_shares_.end());
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


	protected: virtual_machine_monitor_pointer vmm_ptr() const
	{
		return ptr_vmm_;
	}


	protected: virtual_machine_monitor_pointer vmm_ptr()
	{
		return ptr_vmm_;
	}


	private: void create_share_stats(physical_resource_category category)
	{
		typedef ::dcs::eesim::registry<traits_type> registry_type;
		typedef typename registry_type::des_engine_type des_engine_type;

		if (wanted_res_shares_stats_.count(category) && res_shares_stats_.count(category))
		{
			return;
		}

		des_engine_type& eng(registry_type::instance().des_engine());

		// Create share stats for wanted shares (i==0) and real shares (i==1)
		for (short i = 0; i < 2; ++i)
		{
			statistic_container stats;

			stats.push_back(eng.make_analyzable_statistic(::dcs::des::min_estimator<real_type,uint_type>()));
			stats.push_back(eng.make_analyzable_statistic(::dcs::des::quantile_estimator<real_type,uint_type>(0.25)));
			stats.push_back(eng.make_analyzable_statistic(::dcs::des::quantile_estimator<real_type,uint_type>(0.50)));
			stats.push_back(eng.make_analyzable_statistic(::dcs::des::quantile_estimator<real_type,uint_type>(0.75)));
			stats.push_back(eng.make_analyzable_statistic(::dcs::des::max_estimator<real_type,uint_type>()));
			stats.push_back(eng.make_analyzable_statistic(::dcs::des::mean_estimator<real_type,uint_type>()));

			if (i != 0)
			{
				wanted_res_shares_stats_[category] = stats;
			}
			else
			{
				res_shares_stats_[category] = stats;
			}
		}
	}


	private: void update_wanted_share_stats(physical_resource_category category, real_type value)
	{
		if (power_status_ != powered_on_power_status)
		{
			return;
		}

		if (!wanted_res_shares_stats_.count(category))
		{
			create_share_stats(category);
		}

//		::std::for_each(
//				wanted_res_shares_stats_[category].begin(),
//				wanted_res_shares_stats_[category].end(),
//				::dcs::functional::bind(
//					&statistic_type::operator(),
//					::dcs::functional::placeholders::_1,
//					value
//				)
//			);

		// Express the input (actual) share value wrt reference machine
		if (ptr_tier_ && ptr_vmm_)
		{
			value = scale_resource_share(ptr_vmm_->hosting_machine().resource(category)->capacity(),
										 ptr_tier_->application().reference_resource(category).capacity(),
										 value);
		}

		typedef typename statistic_container::iterator iterator;
		iterator end_it(wanted_res_shares_stats_[category].end());
		for (iterator it = wanted_res_shares_stats_[category].begin(); it != end_it; ++it)
		{
			statistic_pointer ptr_stat(*it);

			(*ptr_stat)(value);
		}
	}


	private: template <typename ForwardIterT>
		void update_wanted_share_stats(ForwardIterT first, ForwardIterT last)
	{
		while (first != last)
		{
			update_wanted_share_stats(first->first, first->second);
			++first;
		}
	}


	private: void reset_wanted_share_stats()
	{
		wanted_res_shares_stats_.clear();
	}


	private: void reset_wanted_share_stats(physical_resource_category category)
	{
		if (wanted_res_shares_stats_.count(category))
		{
			::std::for_each(
					wanted_res_shares_stats_[category].begin(),
					wanted_res_shares_stats_[category].end(),
					::dcs::functional::bind(
						&statistic_type::reset(),
						::dcs::functional::placeholders::_1
					)
				);
		}
	}


	private: template <typename ForwardIterT>
		void reset_wanted_share_stats(ForwardIterT first, ForwardIterT last)
	{
		while (first != last)
		{
			update_wanted_share_stats(*first);
			++first;
		}
	}


	private: void update_share_stats(physical_resource_category category, real_type value)
	{
		if (power_status_ != powered_on_power_status)
		{
			return;
		}

		if (!res_shares_stats_.count(category))
		{
			create_share_stats(category);
		}

		// Express the input (actual) share value wrt reference machine
		if (ptr_tier_ && ptr_vmm_)
		{
			value = scale_resource_share(ptr_vmm_->hosting_machine().resource(category)->capacity(),
										 ptr_tier_->application().reference_resource(category).capacity(),
										 value);
		}

		::std::for_each(
				res_shares_stats_[category].begin(),
				res_shares_stats_[category].end(),
				::dcs::functional::bind(
					&statistic_type::operator(),
					::dcs::functional::placeholders::_1,
					value
				)
			);
	}


	private: template <typename ForwardIterT>
		void update_share_stats(ForwardIterT first, ForwardIterT last)
	{
		while (first != last)
		{
			update_share_stats(first->first, first->second);
			++first;
		}
	}


	private: void reset_share_stats()
	{
		res_shares_stats_.clear();
	}


	private: void reset_share_stats(physical_resource_category category)
	{
		if (res_shares_stats_.count(category))
		{
			::std::for_each(
					res_shares_stats_[category].begin(),
					res_shares_stats_[category].end(),
					::dcs::functional::bind(
						&statistic_type::reset(),
						::dcs::functional::placeholders::_1
					)
				);
		}
	}


	private: template <typename ForwardIterT>
		void reset_share_stats(ForwardIterT first, ForwardIterT last)
	{
		while (first != last)
		{
			update_share_stats(*first);
			++first;
		}
	}


	private: identifier_type id_;
	private: ::std::string name_;
//	private: queue_model_type queue_;
	private: power_status power_status_;
	private: resource_share_impl_container res_shares_;
	private: resource_share_impl_container wanted_res_shares_;
	private: application_tier_pointer ptr_tier_;
//	private: physical_machine_pointer ptr_mach_;
//	private: virtual_machine_monitor_pointer ptr_vmm_;
	private: virtual_machine_monitor_pointer ptr_vmm_;
	private: resource_share_stat_impl_container wanted_res_shares_stats_;
	private: resource_share_stat_impl_container res_shares_stats_;
};


template <typename CharT, typename CharTraitsT, typename TraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, virtual_machine<TraitsT> const& vm)
{
	os << "<"
	   <<   "ID: " << vm.id()
	   << ", Name: " << vm.name();
	if (vm.assigned())
	{
	   os << ", Guest: " << vm.guest_system().id();
	}
	else
	{
	   os << ", Guest: <Not Assigned>";
	}
	os << ", Deployed: " << ::std::boolalpha << vm.deployed()
	   << ", Power Status: " << vm.power_state()
	   << ">";

	return os;
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_VIRTUAL_MACHINE_HPP
