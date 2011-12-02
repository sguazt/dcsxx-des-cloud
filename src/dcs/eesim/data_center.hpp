/**
 * \file dcs/eesim/data_center.hpp
 *
 * \brief Models a data center.
 *
 * Copyright (C) 2009-2011  Distributed Computing System (DCS) Group, Computer
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


#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/base_application_controller.hpp>
#include <dcs/eesim/base_physical_machine_controller.hpp>
#include <dcs/eesim/logging.hpp>
#include <dcs/eesim/multi_tier_application.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/power_status.hpp>
#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/exception.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>


namespace dcs { namespace eesim {

#ifdef DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
template <typename TraitsT>
class data_center_manager;
#endif // DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS

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
	private: typedef ::std::map<application_identifier_type,application_pointer> application_container;
	private: typedef ::std::map<application_identifier_type,application_controller_pointer> application_controller_container;
	private: typedef ::std::map<physical_machine_identifier_type,physical_machine_pointer> physical_machine_container;
	private: typedef ::std::map<physical_machine_identifier_type,physical_machine_controller_pointer> physical_machine_controller_container;
	public: typedef virtual_machine<traits_type> virtual_machine_type;
	public: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
#ifdef DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
	public: typedef data_center_manager<traits_type>* manager_pointer;
#endif // DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
	private: typedef ::std::map<virtual_machine_identifier_type,virtual_machine_pointer> virtual_machine_container;
	private: typedef registry<traits_type> registry_type;
	private: typedef typename traits_type::uint_type uint_type;
	private: typedef typename application_type::application_tier_type application_tier_type;
	private: typedef ::dcs::shared_ptr<application_tier_type> application_tier_pointer;
	private: typedef ::std::set<virtual_machine_identifier_type> deployed_application_vm_container;
	private: typedef ::std::map<
						application_identifier_type,
						deployed_application_vm_container
					> deployed_application_container;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef ::std::set<application_identifier_type> application_id_container;


	/// Default constructor
	public: data_center()
	{
		init();
	}


	/// Copy constructor.
	private: data_center(data_center const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(that);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Copy assigment.
	private: data_center& operator=(data_center const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(rhs);

		//TODO
		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}


	/// Destructor
	public: virtual ~data_center()
	{
		finit();
	}


	public: application_identifier_type add_application(application_pointer const& ptr_app,
														application_controller_pointer const& ptr_app_control,
														bool deploy = true)
	{
		// pre: ptr_app must be a valid application pointer.
		DCS_ASSERT(
			ptr_app,
			throw ::std::invalid_argument("[dcs::eesim::add_application] Invalid application.")
		);
		// pre: ptr_app_control must be a valid application controller pointer.
		DCS_ASSERT(
			ptr_app_control,
			throw ::std::invalid_argument("[dcs::eesim::add_application] Invalid application controller.")
		);

		application_identifier_type id;

		id = registry<traits_type>::instance().application_id_generator()();
		apps_[id] = ptr_app;
		app_ctrls_[id] = ptr_app_control;
::std::cerr << "[data_center] Added APPLICATION: " << *ptr_app << " (" << ptr_app << ")" << ::std::endl;///XXX
		ptr_app->id(id);
::std::cerr << "[data_center] Change ID to APPLICATION: " << *ptr_app << " (" << ptr_app << ")" << ::std::endl;///XXX

		if (deploy)
		{
			deploy_application(id);
		}

		ptr_app->data_centre(this);

		return id;
	}


	public: void remove_application(application_identifier_type app_id)
	{
		// pre: app_id must be a valid application identifier.
		DCS_ASSERT(
			apps_.count(app_id) > 0,
			throw ::std::invalid_argument("[dcs::eesim::remove_application] Invalid application identifier.")
		);

		undeploy_application(app_id);

		apps_.erase(app_id);
		app_ctrls_.erase(app_id);
	}


	public: physical_machine_identifier_type add_physical_machine(physical_machine_pointer const& ptr_mach,
																  physical_machine_controller_pointer const& ptr_mach_control)
	{
		// pre: ptr_mach must be a valid physical machine pointer.
		DCS_ASSERT(
			ptr_mach,
			throw ::std::invalid_argument("[dcs::eesim::add_physical_machine] Invalid physical machine.")
		);
		// pre: ptr_mach_control must be a valid physical machine controller pointer.
		DCS_ASSERT(
			ptr_mach_control,
			throw ::std::invalid_argument("[dcs::eesim::add_physical_machine] Invalid physical machine controller.")
		);

		physical_machine_identifier_type id;

		id = registry<traits_type>::instance().physical_machine_id_generator()();
		pms_[id] = ptr_mach;
		pm_ctrls_[id] = ptr_mach_control;
::std::cerr << "[data_center] Added PHYSICAL-MACHINE: " << *ptr_mach << " (" << ptr_mach << ")" << ::std::endl;///XXX
		ptr_mach->id(id);
::std::cerr << "[data_center] Change ID to PHYSICAL-MACHINE: " << *ptr_mach << " (" << ptr_mach << ")" << ::std::endl;///XXX

		return id;
	}


	public: void remove_physical_machine(physical_machine_identifier_type mach_id)
	{
		// pre: mach_id must be a valid physical machine identifier.
		DCS_ASSERT(
			pms_.count(mach_id) > 0,
			throw ::std::invalid_argument("[dcs::eesim::remove_physical_machine] Invalid physical machine identifier.")
		);

		pms_.erase(mach_id);
		pm_ctrls_.erase(mach_id);
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
		// pre: mach_id must be a valid physical machine identifier.
		DCS_ASSERT(
			pms_.count(id) > 0,
			throw ::std::invalid_argument("[dcs::eesim::physical_machine_ptr] Invalid physical machine identifier.")
		);

		return pms_.at(id);
	}


	public: physical_machine_pointer physical_machine_ptr(physical_machine_identifier_type id)
	{
		// pre: mach_id must be a valid physical machine identifier.
		DCS_ASSERT(
			pms_.count(id) > 0,
			throw ::std::invalid_argument("[dcs::eesim::physical_machine_ptr] Invalid physical machine identifier.")
		);

		return pms_[id];
	}


	public: ::std::vector<physical_machine_pointer> physical_machines() const
	{
		typedef ::std::vector<physical_machine_pointer> result_container;
		typedef typename physical_machine_container::const_iterator pm_iterator;

		result_container pms;
		pm_iterator end_it(pms_.end());
		for (pm_iterator it = pms_.begin(); it != end_it; ++it)
		{
			// check: paranoid check
			DCS_DEBUG_ASSERT( it->second );

			pms.push_back(it->second);
		}

		return pms;
	}


	public: ::std::vector<physical_machine_pointer> physical_machines(power_status status) const
	{
		typedef ::std::vector<physical_machine_pointer> result_container;
		typedef typename physical_machine_container::const_iterator pm_iterator;

		result_container pms;

		pm_iterator end_it(pms_.end());
		for (pm_iterator it = pms_.begin(); it != end_it; ++it)
		{
			physical_machine_pointer ptr_pm(it->second);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_pm );

			if (ptr_pm->power_state() == status)
			{
				pms.push_back(ptr_pm);
			}
		}

		return pms;
	}


	public: template <typename UnaryPredicateT>
		::std::vector<physical_machine_pointer> physical_machines(UnaryPredicateT pred) const
	{
		typedef ::std::vector<physical_machine_pointer> result_container;
		typedef typename physical_machine_container::const_iterator pm_iterator;

		result_container pms;

		pm_iterator end_it(pms_.end());
		for (pm_iterator it = pms_.begin(); it != end_it; ++it)
		{
			physical_machine_pointer ptr_pm(it->second);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_pm );

			if (pred(ptr_pm))
			{
				pms.push_back(ptr_pm);
			}
		}

		return pms;
	}


	public: physical_machine_controller_type const& physical_machine_controller(physical_machine_identifier_type id) const
	{
		// pre: id must be a valid physical machine identifier.
		DCS_ASSERT(
			pm_ctrls_.count(id) > 0,
			throw ::std::invalid_argument("[dcs::eesim::physical_machine_controller] Invalid physical machine identifier.")
		);

		return *(pm_ctrls_.at(id));
	}


	public: physical_machine_controller_type& physical_machine_controller(physical_machine_identifier_type id)
	{
		// pre: id must be a valid physical machine identifier.
		DCS_ASSERT(
			pm_ctrls_.count(id) > 0,
			throw ::std::invalid_argument("[dcs::eesim::physical_machine_controller] Invalid physical machine identifier.")
		);

		return *(pm_ctrls_[id]);
	}

	public: physical_machine_controller_pointer physical_machine_controller_ptr(physical_machine_identifier_type id) const
	{
		// pre: id must be a valid physical machine identifier.
		DCS_ASSERT(
			pm_ctrls_.count(id) > 0,
			throw ::std::invalid_argument("[dcs::eesim::physical_machine_controller_ptr] Invalid physical machine identifier.")
		);

		return pm_ctrls_.at(id);
	}


	public: physical_machine_controller_pointer physical_machine_controller_ptr(physical_machine_identifier_type id)
	{
		// pre: id must be a valid physical machine identifier.
		DCS_ASSERT(
			pm_ctrls_.count(id) > 0,
			throw ::std::invalid_argument("[dcs::eesim::physical_machine_controller_ptr] Invalid physical machine identifier.")
		);

		return pm_ctrls_[id];
	}


	public: application_controller_type const& application_controller(application_identifier_type id) const
	{
		// pre: id must be a valid application identifier.
		DCS_ASSERT(
			app_ctrls_.count(id) > 0,
			throw ::std::invalid_argument("[dcs::eesim::application_controller] Invalid application identifier.")
		);

		return *(app_ctrls_.at(id));
	}


	public: application_controller_type& application_controller(application_identifier_type id)
	{
		// pre: id must be a valid application identifier.
		DCS_ASSERT(
			app_ctrls_.count(id) > 0,
			throw ::std::invalid_argument("[dcs::eesim::application_controller] Invalid application identifier.")
		);

		return *(app_ctrls_[id]);
	}


	public: application_controller_pointer application_controller_ptr(application_identifier_type id) const
	{
		// pre: id must be a valid application identifier.
		DCS_ASSERT(
			app_ctrls_.count(id) > 0,
			throw ::std::invalid_argument("[dcs::eesim::application_controller_ptr] Invalid application identifier.")
		);

		return app_ctrls_.at(id);
	}


	public: application_controller_pointer application_controller_ptr(application_identifier_type id)
	{
		// pre: id must be a valid application identifier.
		DCS_ASSERT(
			app_ctrls_.count(id) > 0,
			throw ::std::invalid_argument("[dcs::eesim::application_controller_ptr] Invalid application identifier.")
		);

		return app_ctrls_[id];
	}


	public: ::std::vector<application_pointer> applications() const
	{
		typedef ::std::vector<application_pointer> result_container;
		typedef typename application_container::const_iterator app_iterator;

		result_container apps;

		app_iterator end_it(apps_.end());
		for (app_iterator it = apps_.begin(); it != end_it; ++it)
		{
			application_pointer ptr_app(it->second);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_app );

			apps.push_back(ptr_app);
		}

		return apps;
	}


	public: ::std::vector<application_pointer> active_applications() const
	{
		typedef ::std::vector<application_pointer> result_container;
		typedef typename application_container::const_iterator app_iterator;

		result_container apps;

		app_iterator end_it(apps_.end());
		for (app_iterator it = apps_.begin(); it != end_it; ++it)
		{
			application_pointer ptr_app(it->second);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_app );

			if (this->inhibited_application(ptr_app->id()))
			{
				continue;
			}

			apps.push_back(ptr_app);
		}

		return apps;
	}


	public: void deploy_application(application_identifier_type app_id)
	{
		// pre: app_id must be a valid ID and the related application pointer
		//      must be valid as well.
		DCS_ASSERT(
				apps_.count(app_id) > 0
				&&
				apps_.at(app_id),
				throw ::std::invalid_argument("[dcs::eesim::data_center::deploy_application] Invalid application identifier.")
			);

::std::cerr << "[data_center] BEGIN Deploy application: " << apps_.at(app_id) << ::std::endl;//XXX
		if (deployed(app_id))
		{
			return;
		}

		uint_type ntiers = apps_.at(app_id)->num_tiers();
		for (uint_type t = 0; t < ntiers; ++t)
		{
			application_tier_pointer ptr_tier(apps_[app_id]->tier(t));

			::std::string vm_name = std::string("VM for ") + ptr_tier->name();

			virtual_machine_pointer ptr_vm(new virtual_machine_type(vm_name));

			ptr_vm->guest_system(ptr_tier);

			virtual_machine_identifier_type id;

			id = registry<traits_type>::instance().virtual_machine_id_generator()();

			vms_[id] = ptr_vm;
::std::cerr << "[data_center] Added VIRTUAL-MACHINE: " << *ptr_vm << " (" << ptr_vm << ")" << ::std::endl;///XXX
			ptr_vm->id(id);
::std::cerr << "[data_center] Change ID to VIRTUAL-MACHINE: " << *ptr_vm << " (" << ptr_vm << ")" << ::std::endl;///XXX
			deployed_apps_[app_id].insert(id);
			apps_[app_id]->simulation_model().tier_virtual_machine(ptr_vm);
			//ptr_tier->virtual_machine(ptr_vm);
		}
::std::cerr << "[data_center] END Deploy application: " << apps_.at(app_id) << ::std::endl;//XXX
	}


	public: void undeploy_application(application_identifier_type app_id)
	{
		// pre: app_id must be a valid ID and the related application pointer
		//      must be valid as well.
		DCS_ASSERT(
				apps_.count(app_id) > 0
				&&
				apps_.at(app_id),
				throw ::std::invalid_argument("[dcs::eesim::data_center::undeploy_application] Invalid application identifier.")
			);

		typedef typename deployed_application_container::const_iterator app_iterator;

::std::cerr << "[data_center] BEGIN Undeploy application: " << apps_.at(app_id) << ::std::endl;//XXX
		app_iterator app_it = deployed_apps_.find(app_id);

		// pre: app_id must identify an already deployed application
		DCS_ASSERT(
			app_it != deployed_apps_.end(),
			throw ::std::invalid_argument("[dcs::eesim::data_center::undeploy_application] Cannot undeploy a non-deployed application.")
		);

		// destroy all associated VMs
		typedef typename deployed_application_vm_container::const_iterator vm_iterator;
		vm_iterator vm_end_it(app_it->second.end());
		for (vm_iterator vm_it = app_it->second.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_identifier_type vm_id(*vm_it);

			// check: make sure vm_id is a valid VM identifier.
			DCS_DEBUG_ASSERT( vms_.count(vm_id) > 0 );
			// check: make sure vm_id refer to a valid VM
			DCS_DEBUG_ASSERT( vms_.at(vm_id) );
			// check: double check on VM identifier.
			DCS_DEBUG_ASSERT( vms_.at(vm_id)->id() == vm_id );

			// Displace this VM
			displace_virtual_machine(vms_.at(vm_id));

			// Remove from the VM list
			vms_.erase(vm_id);
		}

		// Undeploy the application
		deployed_apps_.erase(app_id);
::std::cerr << "[data_center] END Undeploy application: " << apps_.at(app_id) << ::std::endl;//XXX
	}


	public: bool deployed(application_identifier_type id) const
	{
		// pre: app_id must be a valid ID and the related application pointer
		//      must be valid as well.
		DCS_ASSERT(
				apps_.count(id) > 0
				&&
				apps_.at(id),
				throw ::std::invalid_argument("[dcs::eesim::data_center::deployed] Invalid application identifier.")
			);

		if (deployed_apps_.find(id) != deployed_apps_.end())
		{
			return true;
		}

		return false;
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
		// pre: id must be a valid VM id
		DCS_ASSERT(
				vms_.count(id) > 0,
				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Invalid virtual machine identifier." )
			);

		return vms_.at(id);
	}


	public: virtual_machine_pointer virtual_machine_ptr(virtual_machine_identifier_type id)
	{
		// pre: id must be a valid VM id
		DCS_ASSERT(
				vms_.count(id) > 0,
				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Invalid virtual machine identifier." )
			);

		return vms_[id];
	}


	public: ::std::vector<virtual_machine_pointer> virtual_machines() const
	{
		typedef ::std::vector<virtual_machine_pointer> result_container;
		typedef typename virtual_machine_container::const_iterator vm_iterator;

		result_container vms;
		vm_iterator end_it(vms_.end());
		for (vm_iterator it = vms_.begin(); it != end_it; ++it)
		{
			virtual_machine_pointer ptr_vm(it->second);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			vms.push_back(ptr_vm);
		}

		return vms;
	}


	public: ::std::vector<virtual_machine_pointer> active_virtual_machines() const
	{
		typedef ::std::vector<virtual_machine_pointer> result_container;
		typedef typename virtual_machine_container::const_iterator vm_iterator;

		result_container vms;
		vm_iterator end_it(vms_.end());
		for (vm_iterator it = vms_.begin(); it != end_it; ++it)
		{
			virtual_machine_pointer ptr_vm(it->second);

			// check: paranoid check
			DCS_DEBUG_ASSERT( ptr_vm );

			if (this->inhibited_virtual_machine(ptr_vm->id()))
			{
				continue;
			}

			vms.push_back(ptr_vm);
		}

		return vms;
	}


	public: ::std::vector<virtual_machine_pointer> application_virtual_machines(application_identifier_type id) const
	{
		typedef typename deployed_application_container::const_iterator app_iterator;
		typedef typename deployed_application_vm_container::const_iterator vm_iterator;

		app_iterator app_it(deployed_apps_.find(id));
		if (app_it == deployed_apps_.end())
		{
			throw ::std::invalid_argument("[dcs::eesim::application_virtual_machines] Invalid application identifier.");
		}

		::std::vector<virtual_machine_pointer> vms;

		vm_iterator vm_end_it(app_it->second.end());
		for (vm_iterator vm_it = app_it->second.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_identifier_type vm_id(*vm_it);

			// check: make sure vm_id is a valid VM identifier.
			DCS_DEBUG_ASSERT( vms_.count(vm_id) > 0 );
			// check: make sure vm_id refer to a valid VM
			DCS_DEBUG_ASSERT( vms_.at(vm_id) );
			// check: double check on VM identifier.
			DCS_DEBUG_ASSERT( vms_.at(vm_id)->id() == vm_id );


			vms.push_back(vms_.at(vm_id));
		}

		return vms;
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

//			// Check if the VM is currently running
//			if (ptr_vm->power_state() != powered_off_power_status)
//			{
//				// Power off this VM and remove from the related PM
//
//				//ptr_vm->power_off();
//				displace_virtual_machine(ptr_vm);
//			}

			// Place the VM on the new PM
			place_virtual_machine(
				ptr_vm,
				ptr_pm,
				it->second.begin(),
				it->second.end(),
				power_on
			);
		}

//		placement_ = placement;
	}


	public: template <typename ForwardIterT>
		void place_virtual_machine(virtual_machine_pointer const& ptr_vm,
								   physical_machine_pointer const& ptr_pm,
								   ForwardIterT first_share,
								   ForwardIterT last_share,
								   bool power_on = false)
	{
		place_virtual_machine(ptr_vm,
							  ptr_pm,
							  first_share,
							  last_share,
							  power_on,
							  true);
	}


	public: template <typename ForwardIterT>
		void migrate_virtual_machine(virtual_machine_pointer const& ptr_vm,
								     physical_machine_pointer const& ptr_pm,
								     ForwardIterT first_share,
								     ForwardIterT last_share)
	{
		// Power-on target physical machine (if needed)
		if (ptr_pm->power_state() != powered_on_power_status)
		{
			ptr_pm->power_on();
		}
		// Peform migration
		if (ptr_vm->vmm().hosting_machine().id() != ptr_pm->id())
		{
			ptr_vm->vmm().migrate(ptr_vm, *ptr_pm);
		}
		// Update placement
		place_virtual_machine(ptr_vm,
							  ptr_pm,
							  first_share,
							  last_share,
							  false,
							  true);
	}


	public: void displace_virtual_machine(virtual_machine_pointer const& ptr_vm, bool power_off = true)
	{
		/// pre: ptr_vm must be a valid pointer
		DCS_ASSERT(
				ptr_vm,
				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Invalid VM pointer." )
			);

		typedef typename virtual_machines_placement_type::const_iterator iterator;
		iterator it = placement_.find(*ptr_vm);
		if (it != placement_.end())
		{
			// safety-check: the identifier of the input VM must be the same of
			//               the one of the retrieved VM.
			DCS_DEBUG_ASSERT( ptr_vm->id() == placement_.vm_id(it) );

			if (power_off)
			{
				physical_machine_identifier_type pm_id(placement_.pm_id(it));

				// paranoid-check: existence
				DCS_DEBUG_ASSERT( pms_.count(pm_id) > 0 );

				physical_machine_pointer ptr_pm(pms_[pm_id]);

				// paranoid-check: null
				DCS_DEBUG_ASSERT( ptr_pm );

				ptr_pm->vmm().power_off(ptr_vm);
				ptr_pm->vmm().destroy_domain(ptr_vm);

#ifdef DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
				ptr_mngr_->migration_controller().notify_vm_destruction(ptr_vm);
#endif // DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
			}

			placement_.displace(*ptr_vm);
		}
	}


	public: void displace_virtual_machines(bool power_off = true)
	{
		typedef typename virtual_machines_placement_type::const_iterator iterator;
		if (power_off)
		{
			iterator end_it = placement_.end();
			for (iterator it = placement_.begin(); it != end_it; ++it)
			{
				virtual_machine_identifier_type vm_id(placement_.vm_id(it));
				physical_machine_identifier_type pm_id(placement_.pm_id(it));

				// paranoid-check: existence
				DCS_DEBUG_ASSERT( pms_.count(pm_id) > 0 );
				// paranoid-check: existence
				DCS_DEBUG_ASSERT( vms_.count(vm_id) > 0 );

				physical_machine_pointer ptr_pm(pms_.at(pm_id));
				virtual_machine_pointer ptr_vm(vms_.at(vm_id));

				// paranoid-check: null
				DCS_DEBUG_ASSERT( ptr_pm );
				// paranoid-check: double check
				DCS_DEBUG_ASSERT( ptr_pm->id() == pm_id );
				// paranoid-check: null
				DCS_DEBUG_ASSERT( ptr_vm );
				// paranoid-check: double check
				DCS_DEBUG_ASSERT( ptr_vm->id() == vm_id );

				ptr_pm->vmm().power_off(ptr_vm);
				ptr_pm->vmm().destroy_domain(ptr_vm);
//				placement_.displace(*ptr_vm);
			}
		}
		placement_.displace_all();
	}


	public: virtual_machines_placement_type const& current_virtual_machines_placement() const
	{
		return placement_;
	}


	public: uint_type start_applications()
	{
		typedef typename deployed_application_container::const_iterator app_iterator;
		//typedef typename ::std::vector<virtual_machine_identifier_type>::const_iterator vm_iterator;
		typedef typename deployed_application_vm_container::const_iterator vm_iterator;

		uint_type started_apps(0);

		app_iterator app_end_it(deployed_apps_.end());
		for (app_iterator app_it(deployed_apps_.begin()); app_it != app_end_it; ++app_it)
		{
			application_identifier_type app_id(app_it->first);

			bool started;

			started = start_application(app_id);

			if (started)
			{
				++started_apps;
			}
		}

		return started_apps;
	}


	public: bool start_application(application_identifier_type app_id)
	{
		// pre: id must be a valid application identifier
		DCS_ASSERT(
				apps_.count(app_id) > 0,
				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Invalid application identifier" )
			);
		DCS_ASSERT(
				deployed_apps_.count(app_id) > 0,
				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Invalid application identifier" )
			);

::std::cerr << "[data_center] BEGIN Start application: " << *(apps_.at(app_id)) << ::std::endl;//XXX
		bool startable(true);

		if (!this->inhibited_application(app_id))
		{
			typedef typename deployed_application_container::const_iterator app_iterator;
			typedef typename deployed_application_vm_container::const_iterator vm_iterator;

			app_iterator app_it(deployed_apps_.find(app_id));

			vm_iterator vm_end_it(app_it->second.end());
			for (vm_iterator vm_it = app_it->second.begin(); startable && vm_it != vm_end_it; ++vm_it)
			{
				startable = placement_.placed(*vm_it);
			}

			if (startable)
			{
	//			this->inhibit_application(app_id, false);

				typedef typename virtual_machines_placement_type::const_iterator vm_placement_iterator;
				::std::vector<virtual_machine_pointer> app_vms;
				for (vm_iterator vm_it = app_it->second.begin(); vm_it != vm_end_it; ++vm_it)
				{
					vm_placement_iterator vm_place_it(placement_.find(*vm_it));
					physical_machine_pointer ptr_pm(pms_[placement_.pm_id(vm_place_it)]);
					virtual_machine_pointer ptr_vm(vms_.at(placement_.vm_id(vm_place_it)));

					// paranoid-check: null
					DCS_DEBUG_ASSERT( ptr_pm );
					// paranoid-check: null
					DCS_DEBUG_ASSERT( ptr_vm );

					ptr_pm->vmm().power_on(ptr_vm);
//					virtual_machine_pointer ptr_vm(vms_[*vm_it]);
//					ptr_vm->power_on();
					app_vms.push_back(ptr_vm);
				}
				this->application_ptr(app_id)->start(app_vms.begin(), app_vms.end());
			}
			else
			{
				::std::ostringstream oss;
				oss << "Application " << app_id << " '" << *(apps_.at(app_id)) << "' cannot be started: at least one VM has not been placed.";

				log_warn(DCS_EESIM_LOGGING_AT, oss.str());
			}
		}
		else
		{
			startable = false;

			::std::ostringstream oss;
			oss << "Application " << app_id << " '" << *(apps_.at(app_id)) << "' cannot be started because it has been inhibited.";

			log_warn(DCS_EESIM_LOGGING_AT, oss.str());
		}

::std::cerr << "[data_center] END Start application: " << *(apps_.at(app_id)) << ::std::endl;//XXX
		return startable;
	}


	public: uint_type stop_applications()
	{
		typedef typename deployed_application_container::const_iterator app_iterator;
		typedef typename deployed_application_vm_container::const_iterator vm_iterator;

		uint_type stopped_apps(0);

		app_iterator app_end_it(deployed_apps_.end());
		for (app_iterator app_it(deployed_apps_.begin()); app_it != app_end_it; ++app_it)
		{
			application_identifier_type app_id(app_it->first);

			bool stopped;

			stopped = stop_application(app_id);

			if (stopped)
			{
				++stopped_apps;
			}
		}

		return stopped_apps;
	}


	public: bool stop_application(application_identifier_type app_id)
	{
		// pre: id must be a valid application identifier
		DCS_ASSERT(
				apps_.count(app_id) > 0,
				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Invalid application identifier" )
			);
		DCS_ASSERT(
				deployed_apps_.count(app_id) > 0,
				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Invalid application identifier" )
			);

::std::cerr << "[data_center] BEGIN Stop application: " << *(apps_.at(app_id)) << ::std::endl;//XXX
		bool stoppable(true);

		if (!this->inhibited_application(app_id))
		{
			typedef typename deployed_application_container::const_iterator app_iterator;
			typedef typename deployed_application_vm_container::const_iterator vm_iterator;

			app_iterator app_it(deployed_apps_.find(app_id));

			vm_iterator vm_end_it(app_it->second.end());
			for (vm_iterator vm_it = app_it->second.begin(); stoppable && vm_it != vm_end_it; ++vm_it)
			{
				stoppable = placement_.placed(*vm_it);
			}

			if (stoppable)
			{
				typedef typename virtual_machines_placement_type::const_iterator vm_placement_iterator;
				for (vm_iterator vm_it = app_it->second.begin(); vm_it != vm_end_it; ++vm_it)
				{
					// paranoid-check: existence
					DCS_DEBUG_ASSERT( vms_.count(*vm_it) > 0 );

					virtual_machine_pointer ptr_vm(vms_.at(*vm_it));

					// paranoid-check: null
					DCS_DEBUG_ASSERT( ptr_vm );

					//vms_.at(*vm_it)->power_off();

					ptr_vm->vmm().power_off(ptr_vm);
				}
				this->application_ptr(app_id)->stop();
			}
			else
			{
				::std::ostringstream oss;
				oss << "Application " << app_id << " '" << *(apps_.at(app_id)) << "' cannot be stopped: at least one VM has not been placed.";

				log_warn(DCS_EESIM_LOGGING_AT, oss.str());
			}
		}
		else
		{
			stoppable = false;

			::std::ostringstream oss;
			oss << "Application " << app_id << " '" << *(apps_.at(app_id)) << "' cannot be stopped because it has been inhibited.";

			log_warn(DCS_EESIM_LOGGING_AT, oss.str());
		}

::std::cerr << "[data_center] END Stop application: " << *(apps_.at(app_id)) << ::std::endl;//XXX
		return stoppable;
	}


	public: void inhibit_application(application_identifier_type id, bool inhibit)
	{
		if (inhibit)
		{
			inhibited_apps_.insert(id);
		}
		else
		{
			inhibited_apps_.erase(id);
		}
		apps_[id]->simulation_model().enable(!inhibit);
		app_ctrls_[id]->enable(!inhibit);
	}


	public: bool inhibited_application(application_identifier_type id) const
	{
		return inhibited_apps_.count(id) > 0;
	}


#ifdef DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
	public: void manager(manager_pointer const& ptr_mngr)
	{
		ptr_mngr_ = ptr_mngr;
	}
#endif // DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS


	private: bool inhibited_virtual_machine(virtual_machine_identifier_type id) const
	{
		return inhibited_apps_.count(vms_.at(id)->guest_system().application().id()) > 0;
	}


	private: void init()
	{
		registry<traits_type>& ref_reg(registry<traits_type>::instance());

		ref_reg.des_engine_ptr()->system_initialization_event_source().connect(
			::dcs::functional::bind(
				&self_type::process_sys_init,
				this,
				::dcs::functional::placeholders::_1,
				::dcs::functional::placeholders::_2
			)
		);
	}


	private: void finit()
	{
		registry<traits_type>& ref_reg(registry<traits_type>::instance());

		ref_reg.des_engine_ptr()->system_initialization_event_source().disconnect(
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
		// pre: id must be a valid application identifier
		DCS_ASSERT(
				apps_.count(id) > 0,
				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Invalid application identifier" )
			);

		return apps_[id];
	}


	protected: application_pointer application_ptr(application_identifier_type id) const
	{
		// pre: id must be a valid application identifier
		DCS_ASSERT(
				apps_.count(id) > 0,
				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Invalid application identifier" )
			);

		return apps_.at(id);
	}


	private: template <typename ForwardIterT>
		void place_virtual_machine(virtual_machine_pointer const& ptr_vm,
								   physical_machine_pointer const& ptr_pm,
								   ForwardIterT first_share,
								   ForwardIterT last_share,
								   bool power_on,
								   bool update_placement)
	{
		// pre: ptr_vm must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_vm );
		// pre: ptr_vm must refer to a valid VM
		DCS_DEBUG_ASSERT( vms_.count(ptr_vm->id()) > 0 );
		// pre: ptr_pm must be a valid pointer
		DCS_DEBUG_ASSERT( ptr_pm );
		// pre: ptr_pm must refer to a valid PM
		DCS_DEBUG_ASSERT( pms_.count(ptr_pm->id()) > 0 );

::std::cerr << "[data_center] BEGIN VM Placement>> VM: " << *ptr_vm << " - PM: " << *ptr_pm << " - SHARE: " << first_share->second << ::std::endl;//XXX
		if (ptr_pm->power_state() != powered_on_power_status)
		{
			ptr_pm->power_on();
		}

		ptr_pm->vmm().create_domain(ptr_vm);

		if (power_on)
		{
			ptr_pm->vmm().power_on(ptr_vm);
		}

		if (update_placement)
		{
			if (placement_.placed(*ptr_vm))
			{
				//placement_.displace(*ptr_vm);
				placement_.replace(*ptr_vm, *ptr_pm, first_share, last_share);
			}
			else
			{
				placement_.place(*ptr_vm, *ptr_pm, first_share, last_share);
			}
		}

		// Assign shares to this VM
		while (first_share != last_share)
		{
			ptr_vm->wanted_resource_share(first_share->first, first_share->second);
			ptr_vm->resource_share(first_share->first, first_share->second);
			++first_share;
		}
::std::cerr << "[data_center] END VM Placement>> VM: " << *ptr_vm << " - PM: " << *ptr_pm << " - SHARE: " << first_share->second << ::std::endl;//XXX
	}


	private: application_container apps_;
	private: application_controller_container app_ctrls_;
	private: physical_machine_container pms_;
	private: physical_machine_controller_container pm_ctrls_;
	private: virtual_machine_container vms_;
	private: virtual_machines_placement_type placement_;
	private: deployed_application_container deployed_apps_;
	private: application_id_container inhibited_apps_;
#ifdef DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
	private: manager_pointer ptr_mngr_;
#endif // DCS_EESIM_EXP_MIGR_CONTROLLER_MONITOR_VMS
}; // data_center


}} // Namespace dcs::eesim


#endif // DCS_EESIM_DATA_CENTER_HPP
