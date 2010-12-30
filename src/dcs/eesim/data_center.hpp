/**
 * \file dcs/eesim/data_center.hpp
 *
 * \brief Models a data center.
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

#ifndef DCS_EESIM_DATA_CENTER_HPP
#define DCS_EESIM_DATA_CENTER_HPP


#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/base_application_controller.hpp>
#include <dcs/eesim/base_physical_machine_controller.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/memory.hpp>
#include <iostream>
#include <vector>


namespace dcs { namespace eesim {


template <typename TraitsT>
class data_center
{
	private: typedef data_center<TraitsT> self_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::application_identifier_type application_identifier_type;
	public: typedef typename traits_type::physical_machine_identifier_type physical_machine_identifier_type;
	public: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
	public: typedef multi_tier_application<traits_type> application_type;
	public: typedef ::dcs::shared_ptr<application_type> application_pointer;
	public: typedef physical_machine<traits_type> physical_machine_type;
	public: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	public: typedef base_application_controller<traits_type> application_controller_type;
	public: typedef base_physical_machine_controller<traits_type> physical_machine_controller_type;
	public: typedef ::dcs::shared_ptr<application_controller_type> application_controller_pointer;
	public: typedef ::dcs::shared_ptr<physical_machine_controller_type> physical_machine_controller_pointer;
	public: typedef virtual_machines_placement<traits_type> virtual_machines_placement_type;
	private: typedef ::std::vector<application_pointer> application_container;
	private: typedef ::std::vector<application_controller_pointer> application_controller_container;
	private: typedef ::std::vector<physical_machine_pointer> physical_machine_container;
	private: typedef ::std::vector<physical_machine_controller_pointer> physical_machine_controller_container;
	private: typedef virtual_machine<traits_type> virtual_machine_type;
	private: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	private: typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;
	private: typedef registry<traits_type> registry_type;
	private: typedef typename traits_type::uint_type uint_type;
	private: typedef typename application_type::application_tier_type application_tier_type;
	private: typedef ::dcs::shared_ptr<application_tier_type> application_tier_pointer;
	private: typedef ::std::map<
						application_identifier_type,
						::std::vector<virtual_machine_identifier_type>
					> deployed_application_container;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;



	public: data_center()
	{
		init();
	}


	public: application_identifier_type add_application(application_pointer const& ptr_app,
												   application_controller_pointer const& ptr_app_control)
	{
		application_identifier_type id;

		id = apps_.size();
		apps_.push_back(ptr_app);
		app_ctrls_.push_back(ptr_app_control);
		apps_.back()->id(id);

		deploy_application(id);

		return id;
	}


	public: physical_machine_identifier_type add_physical_machine(
						physical_machine_pointer const& ptr_mach,
						physical_machine_controller_pointer const& ptr_mach_control)
	{
		physical_machine_identifier_type id;

		id = pms_.size();
		pms_.push_back(ptr_mach);
		pm_ctrls_.push_back(ptr_mach_control);
		pms_.back()->id(id);

		return id;
	}


//FIXME: name-clash between method name and type name
//	public: physical_machine_type const& physical_machine(physical_machine_identifier_type id) const
//	{
//		return *(pms_[id]);
//	}


//FIXME: name-clash between method name and type name
//	public: physical_machine_type& physical_machine(physical_machine_identifier_type id)
//	{
//		return *(pms_[id]);
//	}


	public: physical_machine_pointer physical_machine_ptr(physical_machine_identifier_type id) const
	{
		return pms_[id];
	}


	public: physical_machine_pointer physical_machine_ptr(physical_machine_identifier_type id)
	{
		return pms_[id];
	}


	public: ::std::vector<physical_machine_pointer> physical_machines() const
	{
		return pms_;
	}


	public: ::std::vector<physical_machine_pointer> physical_machines(power_status status) const
	{
		typedef typename physical_machine_container::const_iterator machine_iterator;

		::std::vector<physical_machine_pointer> ptr_machs;

		machine_iterator it_end = pms_.end();
		for (machine_iterator it = pms_.begin(); it != it_end; ++it)
		{
			if ((*it)->power_state() == status)
			{
				ptr_machs.push_back(*it);
			}
		}

		return ptr_machs;
	}


	public: template <typename UnaryPredicateT>
		::std::vector<physical_machine_pointer> physical_machines(UnaryPredicateT pred) const
	{
		typedef typename physical_machine_container::const_iterator machine_iterator;

		::std::vector<physical_machine_pointer> ptr_machs;

		machine_iterator end_it(pms_.end());
		for (machine_iterator it(pms_.begin()); it != end_it; ++it)
		{
			if (pred(*it))
			{
				ptr_machs.push_back(*it);
			}
		}

		return ptr_machs;
	}


	public: ::std::vector<application_pointer> applications() const
	{
		return apps_;
	}


//FIXME: name-clash between method name and type name
//	public: virtual_machine_type const& virtual_machine(virtual_machine_identifier_type id) const
//	{
//		return *(vms_[id]);
//	}


//FIXME: name-clash between method name and type name
//	public: virtual_machine_type& virtual_machine(virtual_machine_identifier_type id)
//	{
//		return *(vms_[id]);
//	}


	public: virtual_machine_pointer virtual_machine_ptr(virtual_machine_identifier_type id) const
	{
		return vms_[id];
	}


	public: virtual_machine_pointer virtual_machine_ptr(virtual_machine_identifier_type id)
	{
		return vms_[id];
	}


	public: ::std::vector<virtual_machine_pointer> virtual_machines() const
	{
		return vms_;
	}


	public: void place_virtual_machines(virtual_machines_placement_type const& placement,
										bool power_on = false)
	{
		typedef typename virtual_machines_placement_type::const_iterator iterator;

		iterator end_it = placement.end();
		for (iterator it = placement.begin(); it != end_it; ++it)
		{
			virtual_machine_pointer ptr_vm = this->virtual_machine_ptr(it->first.first);
			physical_machine_pointer ptr_pm = this->physical_machine_ptr(it->first.second);

			// Check if the VM is currently running
			if (ptr_vm->power_state() != powered_off_power_status)
			{
				// Power off this VM and remove from the related PM

				ptr_vm->power_off();
				displace_virtual_machine(ptr_vm);
			}

			// Place the VM on the new PM
			place_virtual_machine(
				ptr_vm,
				ptr_pm,
				it->second.begin(),
				it->second.end(),
				power_on
			);
		}

		placement_ = placement;
	}


	public: template <typename ForwardIterT>
		void place_virtual_machine(virtual_machine_pointer const& ptr_vm,
								   physical_machine_pointer const& ptr_pm,
								   ForwardIterT first_share,
								   ForwardIterT last_share,
								   bool power_on = false)
	{
		if (ptr_pm->power_state() != powered_on_power_status)
		{
			ptr_pm->power_on();
		}
		ptr_vm->resource_shares(first_share, last_share);
		ptr_pm->vmm().create_domain(ptr_vm);

		if (power_on)
		{
			ptr_pm->vmm().power_on(ptr_vm);
		}
	}


	public: void displace_virtual_machine(virtual_machine_pointer const& ptr_vm)
	{
		typedef typename virtual_machines_placement_type::const_iterator iterator;
		iterator it = placement_.find(*ptr_vm);
		if (it != placement_.end())
		{
			// safety-check: the identifier of the input VM must be the same of
			//               the one of the retrieved VM.
			DCS_DEBUG_ASSERT( ptr_vm->id() == it->first.first );

			physical_machine_identifier_type pm_id(it->first.second);
			physical_machine_pointer ptr_pm(pms_[pm_id]);

			ptr_pm->vmm().power_off(ptr_vm);
			ptr_pm->vmm().destroy_domain(ptr_vm);
			placement_.displace(*ptr_vm);
		}
	}


	public: void displace_virtual_machines()
	{
		typedef typename virtual_machines_placement_type::const_iterator iterator;
		iterator end_it = placement_.end();
		for (iterator it = placement_.begin(); it != end_it; ++it)
		{
			virtual_machine_identifier_type vm_id(it->first.first);
			physical_machine_identifier_type pm_id(it->first.second);
			physical_machine_pointer ptr_pm(pms_[pm_id]);
			virtual_machine_pointer ptr_vm(vms_[vm_id]);

			pms_[pm_id]->vmm().power_off(ptr_vm);
			pms_[pm_id]->vmm().destroy_domain(ptr_vm);
			placement_.displace(*ptr_vm);
		}
	}


	public: virtual_machines_placement_type const& current_virtual_machines_placement() const
	{
		return placement_;
	}


	public: uint_type start_applications()
	{
		typedef typename deployed_application_container::const_iterator app_iterator;
		typedef typename ::std::vector<virtual_machine_identifier_type>::const_iterator vm_iterator;

		uint_type started_apps(0);

		app_iterator app_end_it(deployed_apps_.end());
		for (app_iterator app_it(deployed_apps_.begin()); app_it != app_end_it; ++app_it)
		{
			application_identifier_type app_id(app_it->first);

			vm_iterator vm_end_it(app_it->second.end());
			bool startable(true);
			for (vm_iterator vm_it = app_it->second.begin(); startable && vm_it != vm_end_it; ++vm_it)
			{
				startable = placement_.placed(*vm_it);
			}

			if (startable)
			{
				typedef typename virtual_machines_placement_type::const_iterator vm_placement_iterator;
				for (vm_iterator vm_it = app_it->second.begin(); startable && vm_it != vm_end_it; ++vm_it)
				{
//					vm_placement_iterator vm_place_it(placement_.find(*vm_it));
//					physical_machine_pointer ptr_pm(pms_[placement_.pm_id(vm_place_it->first)]);
//					virtual_machine_pointer ptr_vm(mms_[placement_.vm_id(vm_place_it->first)]);
//					ptr_pm->vmm().power_on(ptr_vm);
					vms_[*vm_it]->power_on();
				}
				this->application_ptr(app_id)->start();
				++started_apps;
			}
			else
			{
				::std::clog << "[Warning] Application " << app_id << " '" << apps_[app_id] << "' cannot be started: at least one VM has not been placed." << ::std::endl;
			}
		}

		return started_apps;
	}


	private: void init()
	{
		registry<traits_type>& ref_reg = registry<traits_type>::instance();

		ref_reg.des_engine_ptr()->system_initialization_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_sys_init,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void process_sys_init(des_event_type const& evt, des_engine_context_type& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( evt );
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		DCS_DEBUG_TRACE("(" << this << ") BEGIN Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");//XXX

		displace_virtual_machines();

		DCS_DEBUG_TRACE("(" << this << ") END Processing SYSTEM-INITIALIZATION (Clock: " << ctx.simulated_time() << ")");//XXX
	}


	protected: application_pointer application_ptr(application_identifier_type id)
	{
		return apps_[id];
	}


	protected: application_pointer application_ptr(application_identifier_type id) const
	{
		return apps_[id];
	}


	private: void deploy_application(application_identifier_type app_id)
	{
		uint_type ntiers = apps_[app_id]->num_tiers();
		for (uint_type t = 0; t < ntiers; ++t)
		{
			application_tier_pointer ptr_tier(apps_[app_id]->tier(t));

			::std::string vm_name = std::string("VM for ") + ptr_tier->name();

			virtual_machine_pointer ptr_vm;

			ptr_vm = ::dcs::make_shared<virtual_machine_type>(vm_name);
			ptr_vm->id(vms_.size());
			ptr_vm->guest_system(ptr_tier);
			vms_.push_back(ptr_vm);
			deployed_apps_[app_id].push_back(ptr_vm->id());
			//ptr_tier->virtual_machine(ptr_vm);
		}
	}


	private: bool deployed(application_identifier_type id) const
	{
		if (deployed_apps_.find(id) != deployed_apps_.end())
		{
			return true;
		}

		return false;
	}


//	private: void start_applications()
//	{
//		typedef typename application_container::size_type size_type;
//
//		size_type n = apps_.size();
//		for (size_type i = 0; i < n; ++i)
//		{
//			application_identifier_type app_id(i);
//
//			if (!deployed(app_id))
//			{
//				virtual_machine_pointer ptr_vm = deploy_application(app_id);
//				provision_vm(ptr_vm);
//				ptr_vm->power_on();
//			}
//		}
//	}


	private: application_container apps_;
	private: application_controller_container app_ctrls_;
	private: physical_machine_container pms_;
	private: physical_machine_controller_container pm_ctrls_;
	private: virtual_machine_container vms_;
	private: virtual_machines_placement_type placement_;
	private: deployed_application_container deployed_apps_;
};


}} // Namespace dcs::eesim


#endif // DCS_EESIM_DATA_CENTER_HPP
