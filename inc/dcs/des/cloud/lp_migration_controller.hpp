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
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_EESIM_LP_MIGRATION_CONTROLLER_HPP
#define DCS_EESIM_LP_MIGRATION_CONTROLLER_HPP

#if 0

#undef DCS_EESIM_LP_MIGRATION_CONTROLLER_BASE_TYPE

#include <dcs/debug.hpp>
#include <dcs/des/engine_traits.hpp>
#include <dcs/eesim/base_migration_controller.hpp>
#include <dcs/eesim/detail/config.hpp>
#if defined(DCS_EESIM_CONFIG_USE_GLPK) && DCS_EESIM_CONFIG_USE_GLPK
# include <glpk/glpk.h>
# define  DCS_EESIM_LP_MIGRATION_CONTROLLER_IMPL_TYPE detail::gplk_migration_controller
#elif defined(DCS_EESIM_CONFIG_USE_LPSOLVE) && DCS_EESIM_CONFIG_USE_LPSOLVE
# include <dcs/optim/lpsolve.hpp>
# define  DCS_EESIM_LP_MIGRATION_CONTROLLER_IMPL_TYPE detail::lpsolve_migration_controller
#else
# error "Unable to use a suitable LP library."
#endif // DCS_EESIM_CONFIG_USE_GLPK
#include <dcs/eesim/performance_measure_category.hpp>
#include <dcs/eesim/power_status.hpp>
#include <dcs/memory.hpp>
#ifdef DCS_DEBUG
# include <sstream>
#endif // DCS_DEBUG
#include <stdexcept>
#include <vector>


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


#if defined(DCS_EESIM_CONFIG_USE_GLPK) && DCS_EESIM_CONFIG_USE_GLPK

template <typename TraitsT>
class gplk_migration_controller
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
	private: typedef physical_machine<traits_type> physical_machine_type;
	private: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	private: typedef virtual_machine<traits_type> virtual_machine_type;
	private: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	private: typedef typename virtual_machine_type::application_tier_type application_tier_type;


	public: gplk_migration_controller()
	: base_type(),
	  ptr_dc_(),
	  ts_(1),
	  ew_(1),
	  mw_(1)
	{
	}


	public: gplk_migration_controller(data_center_pointer const& ptr_data_center)
	: base_type(),
	  ptr_dc_(ptr_data_center),
	  ts_(1),
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
		make_problem();
	}


	private: void make_problem()
	{
		glp_prob* lp = 0;

		lp = glp_create_prob();
		glp_set_prob_name(lp, "migration-controller");
		glp_set_obj_dir(lp, GLP_MIN);
		glp_add_rows(lp, 3);
	}


	private: data_center_pointer ptr_dc_;
	/// The sampling time.
	private: real_type ts_;
	/// Weight for energy consumption.
	private: real_type ew_;
	/// Weight for VM migration.
	private: real_type mw_;
}; // lpsolve_migration_controller

#endif // DCS_EESIM_CONFIG_USE_GLPK


#if defined(DCS_EESIM_CONFIG_USE_LPSOLVE) && DCS_EESIM_CONFIG_USE_LPSOLVE

template <typename TraitsT>
class lpsolve_migration_controller
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef data_center<traits_type> data_center_type;
	public: typedef ::dcs::shared_ptr<data_center_type> data_center_pointer;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef physical_machine<traits_type> physical_machine_type;
	private: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	private: typedef virtual_machine<traits_type> virtual_machine_type;
	private: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	private: typedef typename virtual_machine_type::application_tier_type application_tier_type;


	public: lpsolve_migration_controller()
	: ew_(1),
	  mw_(1)
	{
	}


	public: void make_problem(data_center_pointer const& ptr_data_center)
	{
		typedef ::std::vector<physical_machine_pointer> machine_container;
		typedef typename machine_container::const_iterator machine_iterator;
		typedef ::std::vector<virtual_machine_pointer> vm_container;
		typedef typename vm_container::const_iterator vm_iterator;

//		machine_iterator pm_end_it;

		// Create the set of all physical machines
		machine_container pms(ptr_dc_->physical_machines());

		// Create the set of all virtual machines
		vm_container vms(ptr_dc_->virtual_machines());

		::std::size_t n_pms(pms.size());
		::std::size_t n_vms(vms.size());

//		// Create the set of "active" physical machines.
//		// An active physical machine is a physical machine:
//		// * that is powered-on
//		// * that has at least one running VM
//		machine_container active_pms;
//		active_pms = ptr_dc_->physical_machines(detail::active_machine_predicate<physical_machine_pointer>());

		// Create the set of active virtual machines
		// An active virtual machine is a virtual machine:
		// * that is powered on
		vm_container active_vms;
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			physical_machine_pointer ptr_pm(pms[i]);

			if (ptr_pm->power_state() != powered_on_power_status)
			{
				continue;
			}

			vm_container on_vms = ptr_pm->virtual_machine_monitor().virtual_machines(powered_on_power_status);
			active_vms.insert(active_vms.end(), on_vms.begin(), on_vms.end());
//			vm_iterator vm_end_it = on_vms.end();
//			for (vm_iterator vm_it = on_vms.begin(); vm_it != vm_end_it; ++vm_it)
//			{
//				active_vms.push_back(*vm_it);
//			}
		}

		::dcs::optim::lpsolve::lprec* lp(0);

		// Create the LP model: the binary decision variables x_{ij} represent
		// the possibile assignment of virtual machine VM_j on physical machine
		// PM_i. That is, x_{ij}=1 is VM_j will run on PM_i; otherwise,
		// x_{ij}=0.
		// The number of decision variables is given by the product of the
		// number of currently "active" VMs and the number of all possible PMs
		// (included the one that are currently powered off, since, if needed,
		// they migth be powered on).

//		typename ::dcs::optim::lpsolve::int_type nvars = active_vms.size()*active_pms.size();
		typename ::dcs::optim::lpsolve::int_type nvars = active_vms.size()*pms.size();
		lp = ::dcs::optim::lpsolve::make_lp(0, nvars);
		if (!lp)
		{
			throw ::std::runtime_error("Unable to create the LP model.");
		}
		// Speed-up the row entry mode.
		::dcs::optim::lpsolve::set_add_rowmode(lp, ::dcs::optim::lpsolve::true_value);

		::dcs::optim::lpsolve::set_minim(lp); // minimization direction

#ifdef DCS_DEBUG
		//FIXME: cannot pass DCS_DEBUG_STREAM since it returns a 'basic_ostream'
		//       object, while 'set_outputstream' wants a 'FILE*' object.
		//       So, for now make 'stderr' hard-coded.
		//::dcs::optim::lpsolve::set_outputstream(lp, DCS_DEBUG_STREAM);
		::dcs::optim::lpsolve::set_outputstream(lp, stderr);
		::std::size_t ndigits; // # of digits in nvars
		ndigits = static_cast<std::size_t>(::std::floor(::std::log(nvars))) + 1;

		::dcs::optim::lpsolve::set_lp_name(lp, "Best VM allocations");
#endif
		// Set variables to be binary
		for (typename ::dcs::optim::lpsolve::int_type i = 1; i <= nvars; ++i)
		{
			::dcs::optim::lpsolve::set_binary(lp, i, dcs::optim::lpsolve::true_value);
#ifdef DCS_DEBUG
			::std::ostringstream oss;
			oss << "x" << ::std::setw(ndigits) << ::std::setfill('0') << i;
			::dcs::optim::lpsolve::set_col_name(lp, i, const_cast<char*>(oss.str().c_str()));
#endif
		}

		typename ::dcs::optim::lpsolve::real_type* row;
		row = new ::dcs::optim::lpsolve::real_type[nvars+1]; // Note: #rows = #cols+1

		::std::size_t n_avms(active_vms.size());
		::std::size_t col;

		// Create the objective function:
		//   \sum_{i=1}^M\sum_{j=1}^V[c_1 w_{ij}(k) + c_2 MigrCost(j,M(j,k-1),i)]x_{ij}(k)
		col = 1; // start from column (variable) 1 (column 0 is ignored)
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			// The i-th row represents a machine, while the j-th column is a VM

			physical_machine_pointer ptr_pm(pms[i]);

			for (::std::size_t j = 0; j < n_avms; ++j)
			{
				virtual_machine_pointer ptr_vm(active_vms[j]);
				application_tier_type const& tier(ptr_vm->guest_system());

				real_type u;
				u = tier.application().performance_model().tier_measure(tier.id(), utilization_performance_measure);

				//TODO: scale u according to the capacity of reference machine of the application and the one of the actual machine.
				real_type w(0);
				w = ptr_pm->consumed_energy(u);

				//TODO: add migration costs
				row[col] = ew_*w;

				++col;
			}
		}
		// ... And set it in the problem specification.
		::dcs::optim::lpsolve::set_obj_fn(lp, row);

		// Set the constraints

		::dcs::optim::lpsolve::int_type* colno(0);

		// Set the capacity constraint: \sum_{j \in VMs} d_{ij}x_{ij} \le C_i^{\text{max}}, \forall i \in PMs
		colno = new ::dcs::optim::lpsolve::int_type[n_avms];
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			physical_machine_pointer ptr_pm(pms[i]);

			::dcs::optim::lpsolve::real_type threshold(0.8);//FIXME

			for (::std::size_t j = 0; j < n_avms; ++j)
			{
				virtual_machine_pointer ptr_vm(active_vms[j]);
				application_tier_type const& tier(ptr_vm->guest_system());

				typename ::dcs::optim::lpsolve::real_type u;
				u = tier.application().performance_model().tier_measure(tier.id(), utilization_performance_measure);

				//TODO: scale u according to the capacity of reference machine of the application and the one of the actual machine.
				row[j] = u;
				colno[j] = static_cast< ::dcs::optim::lpsolve::int_type >(i*n_avms+j+1);
			}

			::dcs::optim::lpsolve::add_constraintex(lp, n_avms, row, colno, ::dcs::optim::lpsolve::le_constraint, threshold);
		}
		delete[] colno;

		// Set the "only 1 PM per VM" constraints: \sum_{i \in PMs} x_{ij} = 1, \forall j \in VMs
		colno = new ::dcs::optim::lpsolve::int_type[n_pms];
		for (::std::size_t j = 0; j < n_avms; ++j)
		{
			for (::std::size_t i = 0; i < n_pms; ++i)
			{
				row[i] = 1;
				colno[i] = static_cast< ::dcs::optim::lpsolve::int_type >(i*n_avms+j+1);
			}

			::dcs::optim::lpsolve::add_constraintex(lp, n_pms, row, colno, ::dcs::optim::lpsolve::eq_constraint, 1);
		}
		delete[] colno;


		// Disable row entry mode.
		::dcs::optim::lpsolve::set_add_rowmode(lp, ::dcs::optim::lpsolve::false_value);

#ifdef DCS_DEBUG
		::dcs::optim::lpsolve::print_lp(lp);
#endif // DCS_DEBUG

		::dcs::optim::lpsolve::int_type ret;
		ret = ::dcs::optim::lpsolve::solve(lp);

#ifdef DCS_DEBUG
		DCS_DEBUG_TRACE("Elapsed time: " << ::dcs::optim::lpsolve::time_elapsed(lp));
		DCS_DEBUG_TRACE("Solution status:" << ::dcs::optim::lpsolve::get_statustext(lp, ret));

		if (!ret)
		{
			/* optimal solution found */
			::dcs::optim::lpsolve::print_objective(lp);
			::dcs::optim::lpsolve::print_solution(lp, nvars);
			::dcs::optim::lpsolve::print_constraints(lp, 1);
		}
#endif // DCS_DEBUG

		// Destroy the LP model and clean-up
		delete[] row;
		::dcs::optim::lpsolve::free_lp(&lp);
	}


	/// Weight for energy consumption.
	private: real_type ew_;
	/// Weight for VM migration.
	private: real_type mw_;
}; // lpsolve_migration_controller

#endif // DCS_EESIM_CONFIG_USE_LPSOLVE

}} // Namespace detail::<unnamed>


template <typename TraitsT>
class lp_migration_controller: public base_migration_controller<TraitsT>
{
	private: typedef base_migration_controller<TraitsT> base_type;
	private: typedef DCS_EESIM_LP_MIGRATION_CONTROLLER_IMPL_TYPE<TraitsT> impl_type;
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;
	public: typedef data_center<traits_type> data_center_type;
	public: typedef ::dcs::shared_ptr<data_center_type> data_center_pointer;
	private: typedef typename traits_type::des_engine_type des_engine_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_type des_event_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::engine_context_type des_engine_context_type;
	private: typedef typename ::dcs::des::engine_traits<des_engine_type>::event_source_type des_event_source_type;
	private: typedef physical_machine<traits_type> physical_machine_type;
	private: typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
	private: typedef virtual_machine<traits_type> virtual_machine_type;
	private: typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
	private: typedef typename virtual_machine_type::application_tier_type application_tier_type;


	public: lp_migration_controller()
	: base_type(),
	  ptr_dc_(),
	  ts_(1),
	  ew_(1),
	  mw_(1)
	{
	}


	public: lp_migration_controller(data_center_pointer const& ptr_data_center)
	: base_type(),
	  ptr_dc_(ptr_data_center),
	  ts_(1),
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

//		machine_iterator pm_end_it;

		// Create the set of all physical machines
		machine_container pms(ptr_dc_->physical_machines());

		// Create the set of all virtual machines
		vm_container vms(ptr_dc_->virtual_machines());

		::std::size_t n_pms(pms.size());
		::std::size_t n_vms(vms.size());

//		// Create the set of "active" physical machines.
//		// An active physical machine is a physical machine:
//		// * that is powered-on
//		// * that has at least one running VM
//		machine_container active_pms;
//		active_pms = ptr_dc_->physical_machines(detail::active_machine_predicate<physical_machine_pointer>());

		// Create the set of active virtual machines
		// An active virtual machine is a virtual machine:
		// * that is powered on
		vm_container active_vms;
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			physical_machine_pointer ptr_pm(pms[i]);

			if (ptr_pm->power_state() != powered_on_power_status)
			{
				continue;
			}

			vm_container on_vms = ptr_pm->virtual_machine_monitor().virtual_machines(powered_on_power_status);
			active_vms.insert(active_vms.end(), on_vms.begin(), on_vms.end());
//			vm_iterator vm_end_it = on_vms.end();
//			for (vm_iterator vm_it = on_vms.begin(); vm_it != vm_end_it; ++vm_it)
//			{
//				active_vms.push_back(*vm_it);
//			}
		}

		::dcs::optim::lpsolve::lprec* lp(0);

		// Create the LP model: the binary decision variables x_{ij} represent
		// the possibile assignment of virtual machine VM_j on physical machine
		// PM_i. That is, x_{ij}=1 is VM_j will run on PM_i; otherwise,
		// x_{ij}=0.
		// The number of decision variables is given by the product of the
		// number of currently "active" VMs and the number of all possible PMs
		// (included the one that are currently powered off, since, if needed,
		// they migth be powered on).

//		typename ::dcs::optim::lpsolve::int_type nvars = active_vms.size()*active_pms.size();
		typename ::dcs::optim::lpsolve::int_type nvars = active_vms.size()*pms.size();
		lp = ::dcs::optim::lpsolve::make_lp(0, nvars);
		if (!lp)
		{
			throw ::std::runtime_error("Unable to create the LP model.");
		}
		// Speed-up the row entry mode.
		::dcs::optim::lpsolve::set_add_rowmode(lp, ::dcs::optim::lpsolve::true_value);

		::dcs::optim::lpsolve::set_minim(lp); // minimization direction

#ifdef DCS_DEBUG
		//FIXME: cannot pass DCS_DEBUG_STREAM since it returns a 'basic_ostream'
		//       object, while 'set_outputstream' wants a 'FILE*' object.
		//       So, for now make 'stderr' hard-coded.
		//::dcs::optim::lpsolve::set_outputstream(lp, DCS_DEBUG_STREAM);
		::dcs::optim::lpsolve::set_outputstream(lp, stderr);
		::std::size_t ndigits; // # of digits in nvars
		ndigits = static_cast<std::size_t>(std::floor(std::log(nvars))) + 1;

		::dcs::optim::lpsolve::set_lp_name(lp, "Best VM allocations");
#endif
		// Set variables to be binary
		for (typename ::dcs::optim::lpsolve::int_type i = 1; i <= nvars; ++i)
		{
			::dcs::optim::lpsolve::set_binary(lp, i, dcs::optim::lpsolve::true_value);
#ifdef DCS_DEBUG
			::std::ostringstream oss;
			oss << "x" << std::setw(ndigits) << std::setfill('0') << i;
			::dcs::optim::lpsolve::set_col_name(lp, i, const_cast<char*>(oss.str().c_str()));
#endif
		}

		typename ::dcs::optim::lpsolve::real_type* row;
		row = new ::dcs::optim::lpsolve::real_type[nvars+1]; // Note: #rows = #cols+1

		::std::size_t n_avms(active_vms.size());
		::std::size_t col;

		// Create the objective function ...
		col = 1; // start from column (variable) 1 (column 0 is ignored)
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			physical_machine_pointer ptr_pm(pms[i]);

			for (::std::size_t j = 0; j < n_avms; ++j)
			{
				virtual_machine_pointer ptr_vm(active_vms[j]);
				application_tier_type const& tier(ptr_vm->guest_system());

				real_type u;
				u = tier.application().performance_model().tier_measure(tier.id(), utilization_performance_measure);

				//TODO: scale u according to the capacity of reference machine of the application and the one of the actual machine.
				real_type w(0);
				w = ptr_pm->consumed_energy(u);

				//TODO: add migration costs
				row[col] = ew_*w;

				++col;
			}
		}
		// ... And set it in the problem specification.
		::dcs::optim::lpsolve::set_obj_fn(lp, row);

		// Set the constraints

		::dcs::optim::lpsolve::int_type* colno(0);

		// Set the capacity constraint: \sum_{j \in VMs} u_{ij}x_{ij} \le u_i^{\text{max}}, \forall i \in PMs
		colno = new ::dcs::optim::lpsolve::int_type[n_avms];
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			physical_machine_pointer ptr_pm(pms[i]);

			::dcs::optim::lpsolve::real_type threshold(0.8);//FIXME

			for (::std::size_t j = 0; j < n_avms; ++j)
			{
				virtual_machine_pointer ptr_vm(active_vms[j]);
				application_tier_type const& tier(ptr_vm->guest_system());

				::dcs::optim::lpsolve::real_type u;
				u = tier.application().performance_model().tier_measure(tier.id(), utilization_performance_measure);

				//TODO: scale u according to the capacity of reference machine of the application and the one of the actual machine.
				row[j] = u;
				colno[j] = static_cast< ::dcs::optim::lpsolve::int_type >(i*n_avms+j+1);
			}

			::dcs::optim::lpsolve::add_constraintex(lp, n_avms, row, colno, ::dcs::optim::lpsolve::le_constraint, threshold);
		}
		delete[] colno;

		// Set the "only 1 PM per VM" constraints: \sum_{i \in PMs} x_{ij} = 1, \forall j \in VMs
		colno = new ::dcs::optim::lpsolve::int_type[n_pms];
		for (::std::size_t j = 0; j < n_avms; ++j)
		{
			for (::std::size_t i = 0; i < n_pms; ++i)
			{
				row[i] = 1;
				colno[i] = static_cast< ::dcs::optim::lpsolve::int_type >(i*n_avms+j+1);
			}

			::dcs::optim::lpsolve::add_constraintex(lp, n_pms, row, colno, ::dcs::optim::lpsolve::eq_constraint, 1);
		}
		delete[] colno;


		// Disable row entry mode.
		::dcs::optim::lpsolve::set_add_rowmode(lp, ::dcs::optim::lpsolve::false_value);

#ifdef DCS_DEBUG
		::dcs::optim::lpsolve::print_lp(lp);
#endif // DCS_DEBUG

		::dcs::optim::lpsolve::int_type ret;
		ret = ::dcs::optim::lpsolve::solve(lp);

#ifdef DCS_DEBUG
		DCS_DEBUG_TRACE("Elapsed time: " << ::dcs::optim::lpsolve::time_elapsed(lp));
		DCS_DEBUG_TRACE("Solution status:" << ::dcs::optim::lpsolve::get_statustext(lp, ret));

		if (!ret)
		{
			/* optimal solution found */
			::dcs::optim::lpsolve::print_objective(lp);
			::dcs::optim::lpsolve::print_solution(lp, nvars);
			::dcs::optim::lpsolve::print_constraints(lp, 1);
		}
#endif // DCS_DEBUG

		// Destroy the LP model and clean-up
		delete[] row;
		::dcs::optim::lpsolve::free_lp(&lp);
	}
#endif // DCS_EESIM_CONFIG_USE_LPSOLVE


	private: data_center_pointer ptr_dc_;
	/// The sampling time.
	private: real_type ts_;
	/// Weight for energy consumption.
	private: real_type ew_;
	/// Weight for VM migration.
	private: real_type mw_;
};

}} // Namespace dcs::eesim

#endif // 0

#endif // DCS_EESIM_LP_MIGRATION_CONTROLLER_HPP
