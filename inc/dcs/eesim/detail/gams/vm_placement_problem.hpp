/**
 * \file dcs/eesim/detail/gams/vm_placement_problem.hpp
 *
 * \brief Utilities for VM placement problems in the GAMS mathematical
 *  environment.
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

#ifndef DCS_EESIM_DETAIL_GAMS_VM_PLACEMENT_PROBLEM_HPP
#define DCS_EESIM_DETAIL_GAMS_VM_PLACEMENT_PROBLEM_HPP


#include <cstddef>
#include <dcs/eesim/power_status.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>
#include <dcs/memory.hpp>
#include <dcs/perfeval/energy.hpp>
#include <iosfwd>
#include <map>
#include <utility>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>


namespace dcs { namespace eesim { namespace detail { namespace gams {

namespace detail { namespace /*<unnamed>*/ {

template <typename TraitsT>
::std::string make_initial_vm_placement_problem(data_center<TraitsT> const& dc,
												typename TraitsT::real_type wp,
												typename TraitsT::real_type ws,
												typename TraitsT::real_type ref_penalty,
												::std::map<typename TraitsT::virtual_machine_identifier_type,
														   typename TraitsT::real_type> const& vm_util_map,
												virtual_machines_placement<TraitsT> const& init_guess,
												::std::vector<typename TraitsT::physical_machine_identifier_type>& pm_ids,
												::std::vector<typename TraitsT::virtual_machine_identifier_type>& vm_ids)
{
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef data_center<traits_type> data_center_type;
	typedef typename data_center_type::physical_machine_type pm_type;
	typedef typename data_center_type::physical_machine_pointer pm_pointer;
	typedef typename data_center_type::virtual_machine_type vm_type;
	typedef typename data_center_type::virtual_machine_pointer vm_pointer;
	typedef ::std::vector<pm_pointer> pm_container;
	typedef typename pm_container::const_iterator pm_iterator;
	typedef ::std::vector<vm_pointer> vm_container;
	typedef typename vm_container::const_iterator vm_iterator;
	typedef typename pm_type::resource_type resource_type;
	typedef typename pm_type::resource_pointer resource_pointer;
	typedef typename resource_type::energy_model_type energy_model_type;
	typedef typename ::dcs::perfeval::energy::fan2007_model<typename energy_model_type::real_type> fan2007_energy_model_impl_type;
	typedef typename data_center_type::application_type application_type;
	typedef typename application_type::reference_physical_resource_type reference_resource_type;
	typedef typename traits_type::physical_machine_identifier_type pm_identifier_type;
	typedef typename traits_type::virtual_machine_identifier_type vm_identifier_type;
	typedef typename ::std::vector<pm_identifier_type> pm_identifier_container;
	typedef typename ::std::vector<vm_identifier_type> vm_identifier_container;
	typedef typename ::std::map<pm_identifier_type, ::std::size_t> pm_identifier_index_map;
	typedef typename ::std::map<vm_identifier_type, ::std::size_t> vm_identifier_index_map;

	::std::ostringstream oss;

	oss << "$offlisting" << ::std::endl
		<< "$offsymxref" << ::std::endl
		<< "$offsymlist" << ::std::endl
		<< "$offuelxref" << ::std::endl
		<< "$offuellist" << ::std::endl
		<< "option solprint = off;" << ::std::endl;

	// Create the set of all physical machines
	pm_container pms(dc.physical_machines());

	::std::size_t n_pms(pms.size());

	// Create the set of all virtual machines
	vm_container vms(dc.active_virtual_machines());

	::std::size_t n_vms(vms.size());

	oss << "sets" << ::std::endl
		<< "i 'The set of physical machines' /1*" << n_pms << "/" << ::std::endl
		<< "j 'The set of virtual machines' /1*" << n_vms << "/" << ::std::endl
		<< "pmplbls 'Names of parameters related to each physical machine' / c0, c1, c2, r, S_max, C, U_max /" << ::std::endl
		<< "vmplbls 'Names of parameters related to each virtual machine' / Cr, ur, Sr_min /" << ::std::endl
		<< ";" << ::std::endl;

	pm_ids = pm_identifier_container(n_pms);
	vm_ids = vm_identifier_container(n_vms);
	pm_identifier_index_map pm_id_idx_map;
	vm_identifier_index_map vm_id_idx_map;

	// Power model coefficients, max share and resource capacity
	oss << "parameters" << ::std::endl
		<< "wp 'The weight for the power consumption cost' / " << wp << " /" << ::std::endl
		<< "ws 'The weight for the SLA preservation cost' / " << ws << " /" << ::std::endl
		<< "pmpvals(i,pmplbls) 'Values of parameters related to each physical machine'" << ::std::endl
		<< "vmpvals(j,vmplbls) 'Values of parameters related to each virtual machine'" << ::std::endl
		<< ";" << ::std::endl;
	for (::std::size_t i = 0; i < n_pms; ++i)
	{
		pm_pointer ptr_pm(pms[i]);

		pm_ids[i] = ptr_pm->id();
		pm_id_idx_map[ptr_pm->id()] = i;

		//FIXME: CPU resource category is hard-coded
		resource_pointer ptr_resource(ptr_pm->resource(::dcs::eesim::cpu_resource_category));
		energy_model_type const& energy_model(ptr_resource->energy_model());
		//FIXME: Fan2007 energy model type is hard-coded
		fan2007_energy_model_impl_type const* ptr_energy_model_impl = dynamic_cast<fan2007_energy_model_impl_type const*>(&energy_model);
		if (!ptr_energy_model_impl)
		{
			throw ::std::runtime_error("[dcs::eesim::detail::gams::detail::make_initial_vm_placement_problem_data] Unable to retrieve energy model.");
		}
		oss << "pmpvals('" << (i+1) << "','c0') = " << ptr_energy_model_impl->coefficient(0) << ";" << ::std::endl
			<< "pmpvals('" << (i+1) << "','c1') = " << ptr_energy_model_impl->coefficient(1) << ";" << ::std::endl
			<< "pmpvals('" << (i+1) << "','c2') = " << ptr_energy_model_impl->coefficient(2) << ";" << ::std::endl
			<< "pmpvals('" << (i+1) << "','r') = " << ptr_energy_model_impl->coefficient(3) << ";" << ::std::endl
			<< "pmpvals('" << (i+1) << "','S_max') = " << (1.0-ref_penalty) << ";" << ::std::endl
			<< "pmpvals('" << (i+1) << "','C') = " << (ptr_resource->capacity()*ptr_resource->utilization_threshold()) << ";" << ::std::endl
			<< "pmpvals('" << (i+1) << "','U_max') = " << ptr_resource->utilization_threshold() << ";" << ::std::endl;
	}

	// Reference machine capacity, utilization and min share of tiers
	for (::std::size_t j = 0; j < n_vms; ++j)
	{
		vm_pointer ptr_vm(vms[j]);

		vm_ids[j] = ptr_vm->id();
		vm_id_idx_map[ptr_vm->id()] = j;
		application_type const& app(ptr_vm->guest_system().application());

		//FIXME: CPU resource category is hard-coded
		reference_resource_type const& ref_resource(app.reference_resource(::dcs::eesim::cpu_resource_category));

		oss << "vmpvals('" << (j+1) << "','Cr') = " << (ref_resource.capacity()*ref_resource.utilization_threshold()) << ";" << ::std::endl
			<< "vmpvals('" << (j+1) << "','ur') = " << vm_util_map.at(ptr_vm->id()) << ";" << ::std::endl
			<< "vmpvals('" << (j+1) << "','Sr_min') = " << 0.2 << ";" << ::std::endl; //FIXME: Minimum share is hard-coded
	}

	oss << "scalars" << ::std::endl
    	<< "wwp 'Normalized weight for the power consumption cost'" << ::std::endl
    	<< "wws 'Normalized weight for the SLA violation cost'" << ::std::endl
    	<< "epsilon 'Small numeric constant'" << ::std::endl
		<< "shares_sum 'The aggregated share demand on a given physical machine'" << ::std::endl
    	<< ";" << ::std::endl
		<< "wwp$(wp gt 0) = wp / (" << n_pms << "*smax(i, pmpvals(i,'c0')+pmpvals(i,'c1')+pmpvals(i,'c2')));" << ::std::endl
		<< "wwp$(wp eq 0) = 0;" << ::std::endl
		<< "wws$(ws gt 0) = ws / " << n_vms << ";" << ::std::endl
		<< "wws$(ws eq 0) = 0;" << ::std::endl
		<< "epsilon = 1.0e-5;" << ::std::endl
		<< "binary variables" << ::std::endl
    	<< "x(i)" << ::std::endl
    	<< "y(i,j)" << ::std::endl
    	<< ";" << ::std::endl
		<< "positive variables" << ::std::endl
    	<< "s(i,j) 'Share assigned to each virtual machine on every physical machine'" << ::std::endl
    	<< "u(i) 'Expected utilization on each physical machine'" << ::std::endl
    	<< ";" << ::std::endl
		<< "free variables" << ::std::endl
    	<< "cost 'The objective cost'" << ::std::endl
    	<< ";" << ::std::endl
		<< "s.up(i,j) = 1;" << ::std::endl
		<< "u.up(i) = 1;" << ::std::endl;

	// Provide initial values (useful for local optimization techniques)
	if (!init_guess.empty())
	{
		oss << "x.l(i) = 0;" << ::std::endl
			<< "y.l(i,j) = 0;" << ::std::endl
			<< "s.l(i,j) = 0;" << ::std::endl;

		typedef typename virtual_machines_placement<traits_type>::const_iterator vm_placement_iterator;
		typedef typename virtual_machines_placement<traits_type>::share_const_iterator vm_placement_share_iterator;

		::std::set< ::std::size_t > used_pms;
		vm_placement_iterator vmp_end_it(init_guess.end());
		for (vm_placement_iterator vmp_it = init_guess.begin(); vmp_it != vmp_end_it; ++vmp_it)
		{
			pm_identifier_type pm_id(init_guess.pm_id(vmp_it));
			vm_identifier_type vm_id(init_guess.vm_id(vmp_it));
			::std::size_t pm_idx(pm_id_idx_map[pm_id]+1);
			::std::size_t vm_idx(vm_id_idx_map[vm_id]+1);

			if (used_pms.count(pm_idx) == 0)
			{
				oss << "x.l('" << pm_idx << "') = 1;" << ::std::endl;
				used_pms.insert(pm_idx);
			}

			oss << "y.l('" << pm_idx << "','" << vm_idx << "') = 1;" << ::std::endl;

			bool found(false);
			vm_placement_share_iterator end_share_it(init_guess.shares_end(vmp_it));
			for (vm_placement_share_iterator share_it = init_guess.shares_begin(vmp_it); share_it != end_share_it; ++share_it)
			{
				//FIXME: CPU resource category is hard-coded
				if (init_guess.resource_category(share_it) == ::dcs::eesim::cpu_resource_category)
				{
					oss << "s.l('" << pm_idx << "','" << vm_idx << "') = " << init_guess.resource_share(share_it) << ";" << ::std::endl;
					found = true;
				}
			}

			if (!found)
			{
				throw ::std::runtime_error("[dcs::eesim::detail::gams::make_initial_vm_placement_problem] Incompatible resource categories.");
			}
		}
	}

	oss << "equations" << ::std::endl
		<< "eq_one_vm_per_mach 'Each VM must be placed exactly on a single machine'" << ::std::endl
		<< "eq_vm_on_active_mach1 'Each VM must be placed on active (powered on) machine'" << ::std::endl
		<< "eq_vm_on_active_mach2 'ditto'" << ::std::endl
		<< "eq_valid_vm_share 'The share assigned to a VM on a machine can be greater than 0 only if the VM is really assigned to that machine'" << ::std::endl
		<< "eq_min_vm_share 'The share assigned to a VM on machine must be no less than the min value on the reference machine'" << ::std::endl
		<< "eq_max_aggr_vm_share 'The aggregated share demand on a physical machine must not exceed the maximum value'" << ::std::endl
		<< "eq_valid_util 'Utilization must be in [0,1] interval'" << ::std::endl
		<< "eq_obj 'Objective'" << ::std::endl
		<< ";" << ::std::endl
		//<< "eq_obj .. cost =e= wwp * sum(i, x(i)*(pmpvals(i,'c0') + pmpvals(i,'c1')*sum(j, y(i,j)*vmpvals(j,'ur')*vmpvals(j,'Cr')/(pmpvals(i,'C')*(s(i,j)+epsilon))) + pmpvals(i,'c2')*sum(j, y(i,j)*vmpvals(j,'ur')*vmpvals(j,'Cr')/(pmpvals(i,'C')*(s(i,j)+epsilon)))**pmpvals(i,'r'))) + wws * sum(i, sum(j, y(i,j)*abs(s(i,j)*pmpvals(i,'C')/vmpvals(j,'Cr')-1)**2));" << ::std::endl
		<< "eq_obj .. cost =e= wwp * sum(i, x(i)*(pmpvals(i,'c0') + pmpvals(i,'c1')*sum(j, y(i,j)*vmpvals(j,'ur')*vmpvals(j,'Cr')/(pmpvals(i,'C'))) + pmpvals(i,'c2')*sum(j, y(i,j)*vmpvals(j,'ur')*vmpvals(j,'Cr')/(pmpvals(i,'C')))**pmpvals(i,'r'))) + wws * sum(i, sum(j, y(i,j)*abs(s(i,j)*pmpvals(i,'C')/vmpvals(j,'Cr')-1)**2));" << ::std::endl
		<< "eq_one_vm_per_mach(j) .. sum(i, y(i,j)) =e= 1;" << ::std::endl
		<< "eq_vm_on_active_mach1(i,j) .. y(i,j) =l= x(i);" << ::std::endl
		<< "eq_vm_on_active_mach2(i) .. sum(j, y(i,j)) =g= x(i);" << ::std::endl
		<< "eq_valid_vm_share(i,j) .. s(i,j) =l= y(i,j);" << ::std::endl
		<< "eq_min_vm_share(i,j) .. s(i,j) =g= y(i,j)*vmpvals(j,'Sr_min')*vmpvals(j,'Cr')/pmpvals(i,'C');" << ::std::endl
		<< "eq_max_aggr_vm_share(i) .. sum(j, s(i,j)) =l= pmpvals(i,'S_max');" << ::std::endl
		//<< "eq_valid_util(i) .. sum(j, y(i,j)*vmpvals(j,'ur')*vmpvals(j,'Cr')/(pmpvals(i,'C')*(s(i,j)+epsilon))) =l= pmpvals(i,'U_max');" << ::std::endl
		<< "eq_valid_util(i) .. sum(j, y(i,j)*vmpvals(j,'ur')*vmpvals(j,'Cr')/(pmpvals(i,'C'))) =l= pmpvals(i,'U_max');" << ::std::endl
		<< "model mdl 'Enegy vs Performance cost (initial)' / all /;" << ::std::endl
		<< "mdl.reslim = 1000000;" << ::std::endl
		<< "solve mdl using minlp minimizing cost;" << ::std::endl
		<< "if(mdl.solvestat eq 1," << ::std::endl
		<< "loop(i," << ::std::endl
		<< "shares_sum = sum(j, s.l(i,j));" << ::std::endl
		<< "if(shares_sum > pmpvals(i,'S_max')," << ::std::endl
		<< "loop(j, s.l(i,j) = s.l(i,j)*pmpvals(i,'S_max')/shares_sum);" << ::std::endl
		<< ");" << ::std::endl
		<< ");" << ::std::endl
		<< ");" << ::std::endl
		<< "$set ofile '%system.OFILE%';" << ::std::endl
		<< "file lst / '%ofile%' /;" << ::std::endl
		<< "lst.ap = 1;" << ::std::endl
		<< "lst.pw = 32767;" << ::std::endl
		<< "put lst;" << ::std::endl
		<< "put / '-- [RESULT] --' /;" << ::std::endl
		<< "put / 'solver_status=', mdl.solvestat /;" << ::std::endl
		<< "put / 'model_status=', mdl.modelstat /;" << ::std::endl
		<< "put / 'cost=', cost.l /;" << ::std::endl
		<< "put / 'x=['; loop(i, put x.l(i)::0); put ']'/;" << ::std::endl
		<< "put / 'y=['; loop(i, loop(j, put y.l(i,j)::0); put ';'); put ']'/;" << ::std::endl
		<< "put / 's=['; loop(i, loop(j, put s.l(i,j)::5); put ';'); put ']'/;" << ::std::endl
		<< "put / '-- [/RESULT] --' /;" << ::std::endl
		<< "putclose;" << ::std::endl;

	return oss.str();
}


template <typename TraitsT, typename UtilFwdIterT, typename ShareFwdIterT>
inline
::std::string make_initial_vm_placement_problem(data_center<TraitsT> const& dc,
												typename TraitsT::real_type wp,
												typename TraitsT::real_type ws,
												typename TraitsT::real_type ref_penalty,
//												::std::map<typename TraitsT::virtual_machine_identifier_type,
//														   typename TraitsT::real_type> const& vm_util_map,
												UtilFwdIterT vm_util_first,
												UtilFwdIterT vm_util_last,
												ShareFwdIterT vm_share_first,
												ShareFwdIterT vm_share_last,
												::std::vector<typename TraitsT::physical_machine_identifier_type>& pm_ids,
												::std::vector<typename TraitsT::virtual_machine_identifier_type>& vm_ids)
{
	return make_initial_vm_placement_problem(dc,
											 wp,
											 ws,
											 ref_penalty,
											 vm_util_first,
											 vm_util_last,
											 vm_share_first,
											 vm_share_last,
											 virtual_machines_placement<TraitsT>(),
											 pm_ids, vm_ids);
}


template <typename TraitsT, typename UtilFwdIterT, typename ShareFwdIterT>
::std::string make_vm_placement_problem(data_center<TraitsT> const& dc,
										typename TraitsT::real_type wp,
										typename TraitsT::real_type wm,
										typename TraitsT::real_type ws,
//										::std::map<typename TraitsT::virtual_machine_identifier_type,
//												   typename TraitsT::real_type> const& vm_util_map,
										UtilFwdIterT vm_util_first,
										UtilFwdIterT vm_util_last,
										ShareFwdIterT vm_share_first,
										ShareFwdIterT vm_share_last,
										virtual_machines_placement<TraitsT> const& init_guess,
										::std::vector<typename TraitsT::physical_machine_identifier_type>& pm_ids,
										::std::vector<typename TraitsT::virtual_machine_identifier_type>& vm_ids)
{
	typedef TraitsT traits_type;
	typedef typename traits_type::real_type real_type;
	typedef data_center<traits_type> data_center_type;
	typedef typename data_center_type::physical_machine_type pm_type;
	typedef typename data_center_type::physical_machine_pointer pm_pointer;
	typedef typename data_center_type::virtual_machine_type vm_type;
	typedef typename data_center_type::virtual_machine_pointer vm_pointer;
	typedef ::std::vector<pm_pointer> pm_container;
	typedef typename pm_container::const_iterator pm_iterator;
	typedef ::std::vector<vm_pointer> vm_container;
	typedef typename vm_container::const_iterator vm_iterator;
	typedef typename pm_type::resource_type resource_type;
	typedef typename pm_type::resource_pointer resource_pointer;
	typedef typename resource_type::energy_model_type energy_model_type;
	typedef typename ::dcs::perfeval::energy::fan2007_model<typename energy_model_type::real_type> fan2007_energy_model_impl_type;
	typedef typename data_center_type::application_type application_type;
	typedef typename application_type::reference_physical_resource_type reference_resource_type;
	typedef typename traits_type::physical_machine_identifier_type pm_identifier_type;
	typedef typename traits_type::virtual_machine_identifier_type vm_identifier_type;
	typedef ::std::vector<pm_identifier_type> pm_identifier_container;
	typedef ::std::vector<vm_identifier_type> vm_identifier_container;
	typedef ::std::map<pm_identifier_type, ::std::size_t> pm_identifier_index_map;
	typedef ::std::map<vm_identifier_type, ::std::size_t> vm_identifier_index_map;
	typedef ::std::map<vm_identifier_type, real_type> vm_utilization_container;
	typedef ::std::map<physical_resource_category, real_type> share_container;
	typedef ::std::map<vm_identifier_type, share_container> vm_share_container;

	::std::ostringstream oss;

	oss << "$offlisting" << ::std::endl
		<< "$offsymxref" << ::std::endl
		<< "$offsymlist" << ::std::endl
		<< "$offuelxref" << ::std::endl
		<< "$offuellist" << ::std::endl
		<< "option solprint = off;" << ::std::endl;

	// Create the set of all physical machines
	pm_container pms(dc.physical_machines());

	::std::size_t n_pms(pms.size());

	// Create the set of active virtual machines
	// An active virtual machine is a virtual machine:
	// * that is powered on
	vm_container active_vms;
	for (::std::size_t i = 0; i < n_pms; ++i)
	{
		pm_pointer ptr_pm(pms[i]);

		if (ptr_pm->power_state() != powered_on_power_status)
		{
			continue;
		}

		vm_container on_vms = ptr_pm->vmm().virtual_machines(powered_on_power_status);
		active_vms.insert(active_vms.end(), on_vms.begin(), on_vms.end());
	}

	// Create the VM utilization container
	vm_utilization_container vm_util_map;
	while (vm_util_first != vm_util_last)
	{
		vm_util_map[vm_util_first->first] = vm_util_first->second;
		++vm_util_first;
	}

	// Create the VM share container
	vm_share_container vm_share_map;
	while (vm_share_first != vm_share_last)
	{
		vm_share_map[vm_share_first->first] = share_container(vm_share_first->second.begin(), vm_share_first->second.end());
		++vm_share_first;
	}

	::std::size_t n_vms(active_vms.size());

	oss << "sets" << ::std::endl
		<< "i 'The set of physical machines' /1*" << n_pms << "/" << ::std::endl
		<< "j 'The set of virtual machines' /1*" << n_vms << "/" << ::std::endl
		<< "pmplbls 'Names of parameters related to each physical machine' / c0, c1, c2, r, S_max, C, U_max /" << ::std::endl
		<< "vmplbls 'Names of parameters related to each virtual machine' / Cr, ur, Sr_min, Sr /" << ::std::endl
		<< ";" << ::std::endl;

	pm_ids = pm_identifier_container(n_pms);
	vm_ids = vm_identifier_container(n_vms);
	pm_identifier_index_map pm_id_idx_map;
	vm_identifier_index_map vm_id_idx_map;

	// Power model coefficients, max share and resource capacity
	oss << "parameters" << ::std::endl
		<< "wp 'The weight for the power consumption cost' / " << wp << " /" << ::std::endl
		<< "wm 'The weight for the power consumption cost' / " << wm << " /" << ::std::endl
		<< "ws 'The weight for the SLA preservation cost' / " << ws << " /" << ::std::endl
		<< "pmpvals(i,pmplbls) 'Values of parameters related to each physical machine'" << ::std::endl
		<< "vmpvals(j,vmplbls) 'Values of parameters related to each virtual machine'" << ::std::endl
		<< "mc(i,j) 'The cost for migrating the virtual machine j into the physical machine i'" << ::std::endl
		<< ";" << ::std::endl;
	for (::std::size_t i = 0; i < n_pms; ++i)
	{
		pm_pointer ptr_pm(pms[i]);

		pm_ids[i] = ptr_pm->id();
		pm_id_idx_map[ptr_pm->id()] = i;

		//FIXME: CPU resource category is hard-coded
		resource_pointer ptr_resource(ptr_pm->resource(::dcs::eesim::cpu_resource_category));
		energy_model_type const& energy_model(ptr_resource->energy_model());
		//FIXME: Fan2007 energy model type is hard-coded
		fan2007_energy_model_impl_type const* ptr_energy_model_impl = dynamic_cast<fan2007_energy_model_impl_type const*>(&energy_model);
		if (!ptr_energy_model_impl)
		{
			throw ::std::runtime_error("[dcs::eesim::detail::gams::detail::make_initial_vm_placement_problem_data] Unable to retrieve energy model.");
		}
		oss << "pmpvals('" << (i+1) << "','c0') = " << ptr_energy_model_impl->coefficient(0) << ";" << ::std::endl
			<< "pmpvals('" << (i+1) << "','c1') = " << ptr_energy_model_impl->coefficient(1) << ";" << ::std::endl
			<< "pmpvals('" << (i+1) << "','c2') = " << ptr_energy_model_impl->coefficient(2) << ";" << ::std::endl
			<< "pmpvals('" << (i+1) << "','r') = " << ptr_energy_model_impl->coefficient(3) << ";" << ::std::endl
			<< "pmpvals('" << (i+1) << "','S_max') = " << 1 << ";" << ::std::endl
			<< "pmpvals('" << (i+1) << "','C') = " << (ptr_resource->capacity()*ptr_resource->utilization_threshold()) << ";" << ::std::endl
			<< "pmpvals('" << (i+1) << "','U_max') = " << ptr_resource->utilization_threshold() << ";" << ::std::endl;
	}

	// Reference machine capacity, utilization and min share of tiers
	for (::std::size_t j = 0; j < n_vms; ++j)
	{
		vm_pointer ptr_vm(active_vms[j]);

		vm_ids[j] = ptr_vm->id();
		vm_id_idx_map[ptr_vm->id()] = j;
		application_type const& app(ptr_vm->guest_system().application());

		//FIXME: CPU resource category is hard-coded
		reference_resource_type const& ref_resource(app.reference_resource(::dcs::eesim::cpu_resource_category));

		oss << "vmpvals('" << (j+1) << "','Cr') = " << (ref_resource.capacity()*ref_resource.utilization_threshold()) << ";" << ::std::endl
			<< "vmpvals('" << (j+1) << "','ur') = " << vm_util_map.at(ptr_vm->id()) << ";" << ::std::endl
			<< "vmpvals('" << (j+1) << "','Sr_min') = " << 0.2 << ";" << ::std::endl //FIXME: Minimum share is hard-coded
			<< "vmpvals('" << (j+1) << "','Sr') = " << vm_share_map.at(ptr_vm->id()).at(cpu_resource_category) << ";" << ::std::endl; //FIXME: CPU category is hard-coded
	}

	// Migration costs
	for (::std::size_t i = 0; i < n_pms; ++i)
	{
		pm_pointer ptr_pm(pms[i]);

		for (::std::size_t j = 0; j < n_vms; ++j)
		{
			vm_pointer ptr_vm(active_vms[j]);

			real_type cost(1);

			if (ptr_pm->id() == ptr_vm->vmm().hosting_machine().id())
			{
				cost = 0;
			}

			oss << "mc('" << (i+1) << "','" << (j+1) << "') = " << cost << ";" << ::std::endl;
		}
	}


	oss << "scalars" << ::std::endl
    	<< "wwp 'Normalized weight for the power consumption cost'" << ::std::endl
    	<< "wwm 'Normalized weight for the power consumption cost'" << ::std::endl
    	<< "wws 'Normalized weight for the SLA violation cost'" << ::std::endl
    	<< "epsilon 'Small numeric constant'" << ::std::endl
		<< "shares_sum 'The aggregated share demand on a given physical machine'" << ::std::endl
    	<< ";" << ::std::endl
		<< "wwp$(wp gt 0) = wp / (" << n_pms << "*smax(i, pmpvals(i,'c0')+pmpvals(i,'c1')+pmpvals(i,'c2')));" << ::std::endl
		<< "wwp$(wp eq 0) = 0;" << ::std::endl
		<< "wwm$(wm gt 0) = wm / (" << n_vms << "*smax(i, smax(j, mc(i,j))));" << ::std::endl
		<< "wwm$(wm eq 0) = 0;" << ::std::endl
		<< "wws$(ws gt 0) = ws / " << n_vms << ";" << ::std::endl
		<< "wws$(ws eq 0) = 0;" << ::std::endl
		<< "epsilon = 1.0e-5;" << ::std::endl
		<< "binary variables" << ::std::endl
    	<< "x(i)" << ::std::endl
    	<< "y(i,j)" << ::std::endl
    	<< ";" << ::std::endl
		<< "positive variables" << ::std::endl
    	<< "s(i,j) 'Share assigned to each virtual machine on every physical machine'" << ::std::endl
    	<< "u(i) 'Expected utilization on each physical machine'" << ::std::endl
    	<< ";" << ::std::endl
		<< "free variables" << ::std::endl
    	<< "cost 'The objective cost'" << ::std::endl
    	<< ";" << ::std::endl
		<< "s.up(i,j) = 1;" << ::std::endl
		<< "u.up(i) = 1;" << ::std::endl;

	// Provide initial values (useful for local optimization techniques)
	if (!init_guess.empty())
	{
		oss << "x.l(i) = 0;" << ::std::endl
			<< "y.l(i,j) = 0;" << ::std::endl
			<< "s.l(i,j) = 0;" << ::std::endl;

		typedef typename virtual_machines_placement<traits_type>::const_iterator vm_placement_iterator;
		typedef typename virtual_machines_placement<traits_type>::share_const_iterator vm_placement_share_iterator;

		::std::set< ::std::size_t > used_pms;
		vm_placement_iterator vmp_end_it(init_guess.end());
		for (vm_placement_iterator vmp_it = init_guess.begin(); vmp_it != vmp_end_it; ++vmp_it)
		{
			pm_identifier_type pm_id(init_guess.pm_id(vmp_it));
			vm_identifier_type vm_id(init_guess.vm_id(vmp_it));
			::std::size_t pm_idx(pm_id_idx_map[pm_id]+1);
			::std::size_t vm_idx(vm_id_idx_map[vm_id]+1);

			if (used_pms.count(pm_idx) == 0)
			{
				oss << "x.l('" << pm_idx << "') = 1;" << ::std::endl;
				used_pms.insert(pm_idx);
			}

			oss << "y.l('" << pm_idx << "','" << vm_idx << "') = 1;" << ::std::endl;

			bool found(false);
			vm_placement_share_iterator end_share_it(init_guess.shares_end(vmp_it));
			for (vm_placement_share_iterator share_it = init_guess.shares_begin(vmp_it); share_it != end_share_it; ++share_it)
			{
				//FIXME: CPU resource category is hard-coded
				if (init_guess.resource_category(share_it) == ::dcs::eesim::cpu_resource_category)
				{
					oss << "s.l('" << pm_idx << "','" << vm_idx << "') = " << init_guess.resource_share(share_it) << ";" << ::std::endl;
					found = true;
				}
			}

			if (!found)
			{
				throw ::std::runtime_error("[dcs::eesim::detail::gams::make_initial_vm_placement_problem] Incompatible resource categories.");
			}
		}
	}

	oss << "equations" << ::std::endl
    	<< "eq_one_vm_per_mach 'Each VM must be placed exactly on a single machine'" << ::std::endl
    	<< "eq_vm_on_active_mach1 'Each VM must be placed on active (powered on) machine'" << ::std::endl
    	<< "eq_vm_on_active_mach2 'ditto'" << ::std::endl
    	<< "eq_valid_vm_share 'The share assigned to a VM on a machine can be greater than 0 only if the VM is really assigned to that machine'" << ::std::endl
    	<< "eq_min_vm_share 'The share assigned to a VM on machine must be no less than the min value on the reference machine'" << ::std::endl
    	<< "eq_max_aggr_vm_share 'The aggregated share demand on a physical machine must not exceed the maximum value'" << ::std::endl
    	<< "eq_valid_util 'Utilization must be in [0,1] interval'" << ::std::endl
    	<< "eq_obj 'Objective'" << ::std::endl
    	<< ";" << ::std::endl
		//<< "eq_obj .. cost =e= wwp * sum(i, x(i)*(pmpvals(i,'c0') + pmpvals(i,'c1')*sum(j, y(i,j)*vmpvals(j,'ur')*vmpvals(j,'Cr')/(pmpvals(i,'C')*(s(i,j)+epsilon))) + pmpvals(i,'c2')*sum(j, y(i,j)*vmpvals(j,'ur')*vmpvals(j,'Cr')/(pmpvals(i,'C')*(s(i,j)+epsilon)))**pmpvals(i,'r')))"
		<< "eq_obj .. cost =e= wwp * sum(i, x(i)*(pmpvals(i,'c0') + pmpvals(i,'c1')*sum(j, y(i,j)*vmpvals(j,'ur')*vmpvals(j,'Cr')/(pmpvals(i,'C'))) + pmpvals(i,'c2')*sum(j, y(i,j)*vmpvals(j,'ur')*vmpvals(j,'Cr')/(pmpvals(i,'C')))**pmpvals(i,'r')))"
		<< "                 + wwm * sum(i, sum(j, y(i,j)*mc(i,j)))"
//		<< "                 + wws * sum(i, sum(j, y(i,j)*abs(s(i,j)*pmpvals(i,'C')/vmpvals(j,'Cr')-1)**2));" << ::std::endl
		<< "                 + wws * sum(i, sum(j, y(i,j)*abs(s(i,j)*pmpvals(i,'C')/vmpvals(j,'Cr')-vmpvals(j,'Sr'))**2));" << ::std::endl
		<< "eq_one_vm_per_mach(j) .. sum(i, y(i,j)) =e= 1;" << ::std::endl
		<< "eq_vm_on_active_mach1(i,j) .. y(i,j) =l= x(i);" << ::std::endl
		<< "eq_vm_on_active_mach2(i) .. sum(j, y(i,j)) =g= x(i);" << ::std::endl
		<< "eq_valid_vm_share(i,j) .. s(i,j) =l= y(i,j);" << ::std::endl
		<< "eq_min_vm_share(i,j) .. s(i,j) =g= y(i,j)*vmpvals(j,'Sr_min')*vmpvals(j,'Cr')/pmpvals(i,'C');" << ::std::endl
		<< "eq_max_aggr_vm_share(i) .. sum(j, s(i,j)) =l= pmpvals(i,'S_max');" << ::std::endl
		//<< "eq_valid_util(i) .. sum(j, y(i,j)*vmpvals(j,'ur')*vmpvals(j,'Cr')/(pmpvals(i,'C')*(s(i,j)+epsilon))) =l= pmpvals(i,'U_max');" << ::std::endl
		<< "eq_valid_util(i) .. sum(j, y(i,j)*vmpvals(j,'ur')*vmpvals(j,'Cr')/(pmpvals(i,'C'))) =l= pmpvals(i,'U_max');" << ::std::endl
		<< "model mdl 'Enegy vs Performance cost' / all /;" << ::std::endl
		<< "mdl.reslim = 1000000;" << ::std::endl
		<< "solve mdl using minlp minimizing cost;" << ::std::endl
		// Normalize shares (if needed)
		<< "if(mdl.solvestat eq 1," << ::std::endl
		<< "loop(i," << ::std::endl
		<< "shares_sum = sum(j, s.l(i,j));" << ::std::endl
		<< "if(shares_sum > pmpvals(i,'S_max')," << ::std::endl
		<< "loop(j, s.l(i,j) = s.l(i,j)*pmpvals(i,'S_max')/shares_sum);" << ::std::endl
		<< ");" << ::std::endl
		<< ");" << ::std::endl
		<< ");" << ::std::endl
		// Output results
		<< "$set ofile '%system.OFILE%';" << ::std::endl
		<< "file lst / '%ofile%' /;" << ::std::endl
		<< "lst.ap = 1;" << ::std::endl
		<< "lst.pw = 32767;" << ::std::endl
		<< "put lst;" << ::std::endl
		<< "put / '-- [RESULT] --' /;" << ::std::endl
		<< "put / 'solver_status=', mdl.solvestat /;" << ::std::endl
		<< "put / 'model_status=', mdl.modelstat /;" << ::std::endl
		<< "put / 'cost=', cost.l /;" << ::std::endl
		<< "put / 'x=['; loop(i, put x.l(i)::0); put ']'/;" << ::std::endl
		<< "put / 'y=['; loop(i, loop(j, put y.l(i,j)::0); put ';'); put ']'/;" << ::std::endl
		<< "put / 's=['; loop(i, loop(j, put s.l(i,j)::5); put ';'); put ']'/;" << ::std::endl
		<< "put / '-- [/RESULT] --' /;" << ::std::endl
		<< "putclose;" << ::std::endl;

	return oss.str();
}


template <typename TraitsT, typename UtilFwdIterT, typename ShareFwdIterT>
inline
::std::string make_vm_placement_problem(data_center<TraitsT> const& dc,
										typename TraitsT::real_type wp,
										typename TraitsT::real_type wm,
										typename TraitsT::real_type ws,
//										::std::map<typename TraitsT::virtual_machine_identifier_type,
//												   typename TraitsT::real_type> const& vm_util_map,
										UtilFwdIterT vm_util_first,
										UtilFwdIterT vm_util_last,
										ShareFwdIterT vm_share_first,
										ShareFwdIterT vm_share_last,
										::std::vector<typename TraitsT::physical_machine_identifier_type>& pm_ids,
										::std::vector<typename TraitsT::virtual_machine_identifier_type>& vm_ids)
{
	return make_vm_placement_problem(dc,
									 wp,
									 wm,
									 ws,
									 vm_util_first,
									 vm_util_last,
									 vm_share_first,
									 vm_share_last,
									 virtual_machines_placement<TraitsT>(),
									 pm_ids, vm_ids);
}

}} // Namespace detail::<unnamed>


template <typename TraitsT>
struct vm_placement_problem
{
	typedef TraitsT traits_type;
	typedef ::std::vector<typename traits_type::physical_machine_identifier_type> physical_machine_identifier_container;
	typedef ::std::vector<typename traits_type::virtual_machine_identifier_type> virtual_machine_identifier_container;

	::std::string model;
    physical_machine_identifier_container pm_ids;
    virtual_machine_identifier_container vm_ids;
};


template <typename TraitsT>
inline
vm_placement_problem<TraitsT> make_initial_vm_placement_problem(data_center<TraitsT> const& dc,
																typename TraitsT::real_type wp,
																typename TraitsT::real_type ws,
																typename TraitsT::real_type ref_penalty,
																::std::map<typename TraitsT::virtual_machine_identifier_type,
																		   typename TraitsT::real_type> const& vm_util_map)
{
	vm_placement_problem<TraitsT> problem;

	problem.model = detail::make_initial_vm_placement_problem(dc,
															  wp,
															  ws,
															  ref_penalty,
															  vm_util_map,
															  problem.pm_ids,
															  problem.vm_ids);

	return problem;
}


template <typename TraitsT>
inline
vm_placement_problem<TraitsT> make_initial_vm_placement_problem(data_center<TraitsT> const& dc,
																typename TraitsT::real_type wp,
																typename TraitsT::real_type ws,
																typename TraitsT::real_type ref_penalty,
																::std::map<typename TraitsT::virtual_machine_identifier_type,
																		   typename TraitsT::real_type> const& vm_util_map,
																virtual_machines_placement<TraitsT> const& init_guess)
{
	vm_placement_problem<TraitsT> problem;

	problem.model = detail::make_initial_vm_placement_problem(dc,
															  wp,
															  ws,
															  ref_penalty,
															  vm_util_map,
															  init_guess,
															  problem.pm_ids,
															  problem.vm_ids);

	return problem;
}


template <typename TraitsT, typename UtilFwdIterT, typename ShareFwdIterT>
inline
vm_placement_problem<TraitsT> make_vm_placement_problem(data_center<TraitsT> const& dc,
														typename TraitsT::real_type wp,
														typename TraitsT::real_type wm,
														typename TraitsT::real_type ws,
														UtilFwdIterT vm_util_first,
														UtilFwdIterT vm_util_last,
														ShareFwdIterT vm_share_first,
														ShareFwdIterT vm_share_last)
//														::std::map<typename TraitsT::virtual_machine_identifier_type,
//																   typename TraitsT::real_type> const& vm_util_map,
//														::std::map<typename TraitsT::virtual_machine_identifier_type,
//																   typename TraitsT::real_type> const& vm_share_map)
{
	vm_placement_problem<TraitsT> problem;

	problem.model = detail::make_vm_placement_problem(dc,
													  wp,
													  wm,
													  ws,
													  vm_util_first,
													  vm_util_last,
													  vm_share_first,
													  vm_share_last,
													  problem.pm_ids,
													  problem.vm_ids);
	return problem;
}


template <typename TraitsT, typename UtilFwdIterT, typename ShareFwdIterT>
inline
vm_placement_problem<TraitsT> make_vm_placement_problem(data_center<TraitsT> const& dc,
														typename TraitsT::real_type wp,
														typename TraitsT::real_type wm,
														typename TraitsT::real_type ws,
														UtilFwdIterT vm_util_first,
														UtilFwdIterT vm_util_last,
														ShareFwdIterT vm_share_first,
														ShareFwdIterT vm_share_last,
//														::std::map<typename TraitsT::virtual_machine_identifier_type,
//																   typename TraitsT::real_type> const& vm_util_map,
//														::std::map<typename TraitsT::virtual_machine_identifier_type,
//																   typename TraitsT::real_type> const& vm_share_map,
														virtual_machines_placement<TraitsT> const& init_guess)
{
	vm_placement_problem<TraitsT> problem;

	problem.model = detail::make_vm_placement_problem(dc,
													  wp,
													  wm,
													  ws,
													  vm_util_first,
													  vm_util_last,
													  vm_share_first,
													  vm_share_last,
													  init_guess,
													  problem.pm_ids,
													  problem.vm_ids);
	return problem;
}

}}}} // Namespace dcs::eesim::detail::gams


#endif // DCS_EESIM_DETAIL_GAMS_VM_PLACEMENT_PROBLEM_HPP
