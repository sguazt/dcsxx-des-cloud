/**
 * \file src/dcs/eesim/lp_migration_controller.hpp
 *
 * \brief Migration Controller based on Linear Programming.
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

#ifndef DCS_EESIM_LP_MIGRATION_CONTROLLER_HPP
#define DCS_EESIM_LP_MIGRATION_CONTROLLER_HPP


#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/base_migration_controller.hpp>
#include <dcs/eesim/detail/config.hpp>
#if defined(DCS_EESIM_CONFIG_USE_GLPK) && DCS_EESIM_CONFIG_USE_GLPK
# include <glpk/glpk.h>
#elif defined(DCS_EESIM_CONFIG_USE_LPSOLVE) && DCS_EESIM_CONFIG_USE_LPSOLVE
# include <lpsolve/lp_lib.h>
#else
#error "Unable to use a suitable LP library."
#endif // DCS_EESIM_CONFIG_USE_GLPK
#ifdef DCS_DEBUG
# include <sstream>
#endif // DCS_DEBUG


namespace dcs { namespace eesim {

namespace detail { namespace /*<unnamed>*/ {

template <typename MachinePointerT>
struct active_machine_predicate: ::std::unary_function<MachinePointerT,bool>
{
	bool operator()(MachinePointerT const& ptr_mach) const
	{
		return ptr_mach->power_state() == powered_on_power_status
			   &&
			   ptr_mach->virtual_machine_monitor().virtual_machines(powered_on_power_status).size() > 0;
	}
};

}} // Namespace detail::<unnamed>


template <typename TraitsT>
class lp_migration_controller: public base_migration_controller<TraitsT>
{
	private: typedef base_migration_controller<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef data_center<traits_type> data_center_type;
	public: typedef ::dcs::shared_ptr<data_center_type> data_center_pointer;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;


	public: lp_migration_controller()
	: base_type(),
	  ptr_dc_(),
	  ew_(1),
	  mw_(1)
	{
	}


	public: lp_migration_controller(data_center_pointer const& ptr_data_center)
	: base_type(),
	  ptr_dc_(ptr_data_center),
	  ew_(1),
	  mw_(1)
	{
	}


	public: void controlled_data_center(data_center_pointer const& ptr_data_center)
	{
		ptr_dc_ = ptr_data_center;
	}


	public: data_center_type const& controlled_data_center() const
	{
		return *ptr_dc_;
	}


	private: void schedule_control()
	{
	}


	private: void process_control(des_event_type const& evt, des_engine_context_type& ctx)
	{
	}


	private: void make_problem()
	{
#if defined(DCS_EESIM_CONFIG_USE_GLPK) && DCS_EESIM_CONFIG_USE_GLPK
		glpk_make_problem();
#elif defined(DCS_EESIM_CONFIG_USE_LPSOLVE) && DCS_EESIM_CONFIG_USE_LPSOLVE
		lpsolve_make_problem();
#endif // DCS_EESIM_CONFIG_USE_GLPK
	}


#if defined(DCS_EESIM_CONFIG_USE_GLPK) && DCS_EESIM_CONFIG_USE_GLPK
	private: void glpk_make_problem()
	{
		glp_prob* lp = 0;

		lp = glp_create_prob();
		glp_set_prob_name(lp, "migration-controller");
		glp_set_obj_dir(lp, GLP_MIN);
		glp_add_rows(lp, 3);
	}
#endif // DCS_EESIM_CONFIG_USE_GLPK


#if defined(DCS_EESIM_CONFIG_USE_LPSOLVE) && DCS_EESIM_CONFIG_USE_LPSOLVE
	private: void lpsolve_make_problem()
	{
		typedef ::std::vector<physical_machine_pointer> machine_container;
		typedef typename machine_container::const_iterator machine_iterator;
		typedef ::std::vector<virtual_machine_pointer> vm_container;
		typedef typename vm_container::const_iterator vm_iterator;

		machine_iterator mach_it_end;

		// Create the set of all physical machines
		machine_container ptr_machines;
		machs = ptr_dc_->physical_machines();

		// Create the set of "active" physical machines.
		// An active physical machine is a physical machine:
		// * that is powered-on
		// * that has at least one running VM
		machine_container active_machs;
		active_machs = ptr_dc_->physical_machines(detail::active_machine_predicate<physical_machine_pointer>());

		/// Create the set of running virtual machines
		vm_container running_vms;
		mach_it_end = active_machs.end();
		for (machine_iterator mach_it = active_machs.begin(); mach_it != mach_it_end; ++mach_it)
		{
			vm_container vms = (*mach_it)->virtual_machine_monitor().virtual_machines(powered_on_power_status);
			vm_iterator vm_it_end = vms.end();
			for (vm_iterator vm_it = vms.begin(); vm_it != vm_it_end; ++vm_it)
			{
				running_vms.push_back(*vm_it);
			}
		}

		::dcs::optim::lpsolve::lprec* lp = 0;

		// Create the LP model.
		typename ::dcs::optim::lpsolve::int_type nvars = active_vms.size()*active_machs.size();
		lp = ::dcs::optim::lpsolve::make_lp(0, nvars);
		if (!lp)
		{
			throw ::std::runtime_error("Unable to create the LP model.");
		}

		::dcs::optim::lpsolve::set_minim(lp); // minimization direction

#ifdef DCS_DEBUG
		::std::size_t ndigits; // # of digits in nvars
		ndigits = static_cast<std::size_t>(std::floor(std::log(n))) + 1;

		::dcs::optim::lpsolve::set_lp_name(lp, "Best VM allocations");
#endif
		// Set variables to be binary
		for (typename ::dcs::optim::lpsolve::int_type i = 1; i <= nvars; ++it)
		{
			::dcs::optim::lpsolve::set_binary(lp, i, dcs::optim::lpsolve::true_value);
#ifdef DCS_DEBUG
			::std::ostringstream oss;
			oss << "x" << std::setw(ndigits) << std::setfill('0') << i;
			::dcs::optim::lpsolve::set_col_name(lp, i, const_cast<char*>(oss.str().c_str()));
#endif
		}

		typename lpsolve::real_type* row;
		row = new lpsolve::real_type[ncols+1]; // Note: #rows = #cols+1

		::std::size_t col = 1; // start from row 1 (row 0 is ignored)
		mach_it_end = active_machs.end();
		for (machine_iterator it = active_machs.begin(); it != mach_it_end; ++it)
		{
			typedef ::std::vector<physical_resource_pointer> resource_container;
			typedef typename resource_container::const_iterator resource_iterator;
			resource_container ress = (*it)->resources();
			resource_iterator res_it_end = 
			row[col] = ew_*(*it)->resources
			++col;
		}


		// Set the objective function
		lpsolve::set:obj_fun(lp, row);

		delete[] row;

		// Destroy the LP model
		free_lp(lp)
	}
#endif // DCS_EESIM_CONFIG_USE_LPSOLVE


	private: data_center_pointer ptr_dc_;
	/// Weight for energy consumption.
	private: real_type ew_;
	/// Weight for VM migration.
	private: real_type mw_;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_LP_MIGRATION_CONTROLLER_HPP
