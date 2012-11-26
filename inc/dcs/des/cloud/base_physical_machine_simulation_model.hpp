/**
 * \file dcs/eesim/base_physical_machine_simulation_model.hpp
 *
 * \brief Base class for physical machine simulation models.
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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_EESIM_BASE_PHYSICAL_MACHINE_SIMULATION_MODEL_HPP
#define DCS_EESIM_BASE_PHYSICAL_MACHINE_SIMULATION_MODEL_HPP


#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/des/entity.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/macro.hpp>
#include <dcs/memory.hpp>


namespace dcs { namespace eesim {

template <typename TraitsT>
struct virtual_machine_migration_context
{
	typedef TraitsT traits_type;
	typedef typename traits_type::physical_machine_identifier_type physical_machine_identifier_type;
	typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;

	/// The identifier of the migrating VM.
	virtual_machine_identifier_type vm_id;
	/// The identifier of the physical machine involved in the migration.
	physical_machine_identifier_type pm_id;
	/// Flag telling if the involved physical machine is the source or
	/// destination one.
	bool pm_is_source;
}; // virtual_machine_migration_context


template <typename TraitsT>
class base_physical_machine_simulation_model: public ::dcs::des::entity
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	public: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	public: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	public: typedef ::dcs::des::base_statistic<real_type,uint_type> output_statistic_type;
	public: typedef ::dcs::shared_ptr<output_statistic_type> output_statistic_pointer;
	public: typedef physical_machine<traits_type> physical_machine_type;
	public: typedef physical_machine_type* physical_machine_pointer;
	public: typedef virtual_machine<traits_type> virtual_machine_type;
	public: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;


	public: virtual ~base_physical_machine_simulation_model()
	{
	}


	public: void machine(physical_machine_pointer const& ptr_mach)
	{
		DCS_DEBUG_ASSERT( ptr_mach );

		ptr_mach_ = ptr_mach;
	}


	public: physical_machine_type& machine()
	{
		// pre: pointer to physical machine is a valid pointer
		DCS_DEBUG_ASSERT( ptr_mach_ );

		return *ptr_mach_;
	}


	public: physical_machine_type const& machine() const
	{
		// pre: pointer to physical machine is a valid pointer
		DCS_DEBUG_ASSERT( ptr_mach_ );

		return *ptr_mach_;
	}


	public: power_status power_state() const
	{
		return do_power_state();
	}


	public: void power_on()
	{
		do_power_on();

		this->enable(true);
	}


	public: void power_off()
	{
		do_power_off();

		this->enable(false);
	}


	public: power_status vm_power_state(virtual_machine_pointer const& ptr_vm) const
	{
		return do_vm_power_state(ptr_vm);
	}


	public: void vm_power_on(virtual_machine_pointer const& ptr_vm)
	{
		do_vm_power_on(ptr_vm);
	}


	public: void vm_power_off(virtual_machine_pointer const& ptr_vm)
	{
		do_vm_power_off(ptr_vm);
	}


	public: void vm_suspend(virtual_machine_pointer const& ptr_vm)
	{
		do_vm_suspend(ptr_vm);
	}


	public: void vm_resume(virtual_machine_pointer const& ptr_vm)
	{
		do_vm_resume(ptr_vm);
	}


	public: void vm_migrate(virtual_machine_pointer const& ptr_vm, physical_machine_type& pm, bool pm_is_source)
	{
		do_vm_migrate(ptr_vm, pm, pm_is_source);
	}


	public: void vm_resource_share(virtual_machine_type const& vm, physical_resource_category category, real_type share)
	{
		do_vm_resource_share(vm, category, share);
	}


	public: des_event_source_type& power_on_event_source()
	{
		return do_power_on_event_source();
	}


	public: des_event_source_type const& power_on_event_source() const
	{
		return do_power_on_event_source();
	}


	public: des_event_source_type& power_off_event_source()
	{
		return do_power_off_event_source();
	}


	public: des_event_source_type const& power_off_event_source() const
	{
		return do_power_off_event_source();
	}


	public: des_event_source_type& vm_power_on_event_source()
	{
		return do_vm_power_on_event_source();
	}


	public: des_event_source_type const& vm_power_on_event_source() const
	{
		return do_vm_power_on_event_source();
	}


	public: des_event_source_type& vm_power_off_event_source()
	{
		return do_vm_power_off_event_source();
	}


	public: des_event_source_type const& vm_power_off_event_source() const
	{
		return do_vm_power_off_event_source();
	}


	public: des_event_source_type& vm_suspend_event_source()
	{
		return do_vm_suspend_event_source();
	}


	public: des_event_source_type const& vm_suspend_event_source() const
	{
		return do_vm_suspend_event_source();
	}


	public: des_event_source_type& vm_resume_event_source()
	{
		return do_vm_resume_event_source();
	}


	public: des_event_source_type const& vm_resume_event_source() const
	{
		return do_vm_resume_event_source();
	}


	public: des_event_source_type& vm_migrate_event_source()
	{
		return do_vm_migrate_event_source();
	}


	public: des_event_source_type const& vm_migrate_event_source() const
	{
		return do_vm_migrate_event_source();
	}


	public: output_statistic_type const& consumed_energy() const
	{
		return do_consumed_energy();
	}


	public: output_statistic_type const& uptime() const
	{
		return do_uptime();
	}


	public: output_statistic_type const& utilization() const
	{
		return do_utilization();
	}


	public: output_statistic_type const& share() const
	{
		return do_share();
	}


	protected: virtual void do_enable(bool flag)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(flag);
	}

	private: virtual power_status do_power_state() const = 0;


	private: virtual void do_power_on() = 0;


	private: virtual void do_power_off() = 0;


	private: virtual power_status do_vm_power_state(virtual_machine_pointer const& ptr_vm) const = 0;


	private: virtual void do_vm_power_on(virtual_machine_pointer const& ptr_vm) = 0;


	private: virtual void do_vm_power_off(virtual_machine_pointer const& ptr_vm) = 0;


	private: virtual void do_vm_suspend(virtual_machine_pointer const& ptr_vm) = 0;


	private: virtual void do_vm_resume(virtual_machine_pointer const& ptr_vm) = 0;


	private: virtual void do_vm_migrate(virtual_machine_pointer const& ptr_vm, physical_machine_type& pm, bool pm_is_source) = 0;


	private: virtual void do_vm_resource_share(virtual_machine_type const& vm, physical_resource_category category, real_type share) = 0;


	private: virtual des_event_source_type& do_power_on_event_source() = 0;


	private: virtual des_event_source_type const& do_power_on_event_source() const = 0;


	private: virtual des_event_source_type& do_power_off_event_source() = 0;


	private: virtual des_event_source_type const& do_power_off_event_source() const = 0;


	private: virtual des_event_source_type& do_vm_power_on_event_source() = 0;


	private: virtual des_event_source_type const& do_vm_power_on_event_source() const = 0;


	private: virtual des_event_source_type& do_vm_power_off_event_source() = 0;


	private: virtual des_event_source_type const& do_vm_power_off_event_source() const = 0;


	private: virtual des_event_source_type& do_vm_suspend_event_source() = 0;


	private: virtual des_event_source_type const& do_vm_suspend_event_source() const = 0;


	private: virtual des_event_source_type& do_vm_resume_event_source() = 0;


	private: virtual des_event_source_type const& do_vm_resume_event_source() const = 0;


	private: virtual des_event_source_type& do_vm_migrate_event_source() = 0;


	private: virtual des_event_source_type const& do_vm_migrate_event_source() const = 0;


	private: virtual output_statistic_type const& do_consumed_energy() const = 0;


	private: virtual output_statistic_type const& do_uptime() const = 0;


	private: virtual output_statistic_type const& do_utilization() const = 0;


	private: virtual output_statistic_type const& do_share() const = 0;


	private: physical_machine_pointer ptr_mach_;
}; // base_physical_machine_simulation_model

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_PHYSICAL_MACHINE_SIMULATION_MODEL_HPP
