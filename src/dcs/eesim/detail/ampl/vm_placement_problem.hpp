#ifndef DCS_EESIM_DETAIL_AMPL_VM_PLACEMENT_PROBLEM_HPP
#define DCS_EESIM_DETAIL_AMPL_VM_PLACEMENT_PROBLEM_HPP


#include <cstddef>
#include <dcs/eesim/power_status.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>
#include <dcs/memory.hpp>
#include <dcs/perfeval/energy.hpp>
#include <iosfwd>
#include <map>
#include <set>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>


namespace dcs { namespace eesim { namespace detail { namespace ampl {

namespace detail { namespace /*<unnamed>*/ {

inline
::std::string make_initial_vm_placement_problem_model()
{
	::std::ostringstream oss;

//	oss << "reset;"
//		<< "model;"
	oss << "param ni > 0 integer;"
		<< "param nj > 0 integer;"
		<< "set I := 1..ni;"
		<< "set J := 1..nj;"
		<< "param c0{I};"
		<< "param c1{I};"
		<< "param c2{I};"
		<< "param r{I};"
		//<< "param ur{J} >= 0, <= 1;"
		<< "param ur{J} >= 0;"
		<< "param Cr{J} >= 0;"
		//<< "param Smax{I} >= 0, <= 1;"
		<< "param Smax{I} >= 0;"
		//<< "param Srmin{J} >= 0, <= 1;"
		<< "param Srmin{J} >= 0;"
		<< "param C{I} >= 0;"
		<< "param Umax{I} >= 0;"
		<< "param wp >= 0 default 0;"
		<< "param ws >= 0 default 0;"
		<< "param eps := 1.0e-5;"
		<< "param wwp := wp / (ni * max{i in I} (c0[i]+c1[i]+c2[i]));"
		<< "param wws := ws / nj;"
		<< "param shares_sum;"
		<< "var x{I} binary;"
		<< "var y{I,J} binary;"
		<< "var s{I,J} >= 0, <= 1;"
		// NOTE: we use term "c2[i]*(eps+sum...) instead of "c2[i]*sum..." since
		// AMPL claims that the value of pow(0,0.4001) cannot be evaluated.
//		<< "minimize cost: wwp * sum{i in I} x[i]*(c0[i] + c1[i]*(sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps))) + c2[i]*(sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps)))^r[i]) + wws * sum{i in I, j in J} ((s[i,j]*C[i]/Cr[j]-1)^2)*y[i,j];"
		//<< "minimize cost: wwp * sum{i in I} x[i]*(c0[i] + c1[i]*(sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps))) + c2[i]*(sum{j in J} (y[i,j]+eps)*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps)))^r[i]) + wws * sum{i in I, j in J} ((s[i,j]*C[i]/Cr[j]-1)^2)*y[i,j];"
		//<< "minimize cost: wwp * sum{i in I} x[i]*(c0[i] + c1[i]*(sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps))) + c2[i]*(eps+sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps)))^r[i]) + wws * sum{i in I, j in J} ((s[i,j]*C[i]/Cr[j]-1)^2)*y[i,j];"
		<< "minimize cost: wwp * sum{i in I} x[i]*(c0[i] + c1[i]*(sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i])) + c2[i]*(eps+sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]))^r[i]) + wws * sum{i in I, j in J} ((s[i,j]*C[i]/Cr[j]-1)^2)*y[i,j];"
		<< "subject to one_vm_per_mach{j in J}: sum{i in I} y[i,j] = 1;"
		<< "subject to vm_on_active_mach1{i in I, j in J}: y[i,j] <= x[i];"
		<< "subject to vm_on_active_mach2{i in I}: sum{j in J} y[i,j] >= x[i];"
		<< "subject to valid_vm_share{i in I, j in J}: s[i,j] <= y[i,j];"
		<< "subject to min_vm_share{i in I, j in J}: s[i,j] >= y[i,j]*Srmin[j]*Cr[j]/C[i];"
		<< "subject to max_aggr_vm_share{i in I}: sum{j in J} s[i,j] <= Smax[i];"
		//<< "subject to valid_util{i in I}: sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps)) <= Umax[i];"
		<< "subject to valid_util{i in I}: sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]) <= Umax[i];"
		<< ::std::endl;

	return oss.str();
}


template <typename TraitsT>
::std::string make_initial_vm_placement_problem_data(data_center<TraitsT> const& dc,
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

	::std::ostringstream oss;

	// Create the set of all physical machines
	pm_container pms(dc.physical_machines());

	::std::size_t n_pms(pms.size());

	// # of machines
	oss << "param ni := " << n_pms << ";" << ::std::endl;

	// Create the set of all virtual machines
	vm_container vms(dc.active_virtual_machines());

	::std::size_t n_vms(vms.size());

	pm_ids = pm_identifier_container(n_pms);
	vm_ids = vm_identifier_container(n_vms);

	// # of VMs
	oss << "param nj := " << n_vms << ";" << ::std::endl;

	// Power model coefficients, max share, resource capacity, and utilization threshold
	oss << "param: c0 c1 c2 r Smax C Umax := " << ::std::endl;
	for (::std::size_t i = 0; i < n_pms; ++i)
	{
		pm_pointer ptr_pm(pms[i]);

		pm_ids[i] = ptr_pm->id();

		//FIXME: CPU resource category is hard-coded
		resource_pointer ptr_resource(ptr_pm->resource(::dcs::eesim::cpu_resource_category));
		energy_model_type const& energy_model(ptr_resource->energy_model());
		//FIXME: Fan2007 energy model type is hard-coded
		fan2007_energy_model_impl_type const* ptr_energy_model_impl = dynamic_cast<fan2007_energy_model_impl_type const*>(&energy_model);
		if (!ptr_energy_model_impl)
		{
			throw ::std::runtime_error("[dcs::eesim::detail::ampl::detail::make_initial_vm_placement_problem_data] Unable to retrieve energy model.");
		}
		oss << (i+1)
			<< " " << ptr_energy_model_impl->coefficient(0) // c0
			<< " " << ptr_energy_model_impl->coefficient(1) // c1
			<< " " << ptr_energy_model_impl->coefficient(2) // c2
			<< " " << ptr_energy_model_impl->coefficient(3) // r
			<< " " << (1.0-ref_penalty) // Smax
			<< " " << (ptr_resource->capacity()*ptr_resource->utilization_threshold()) // C
			<< " " << ptr_resource->utilization_threshold() // Umax
			<< ::std::endl;
	}
	oss << ";";// << ::std::endl;

	// Reference machine capacity, utilization and min share of tiers
	oss << "param: Cr ur Srmin := " << ::std::endl;
	for (::std::size_t j = 0; j < n_vms; ++j)
	{
		vm_pointer ptr_vm(vms[j]);

		vm_ids[j] = ptr_vm->id();
		application_type const& app(ptr_vm->guest_system().application());

		//FIXME: CPU resource category is hard-coded
		reference_resource_type const& ref_resource(app.reference_resource(::dcs::eesim::cpu_resource_category));

		oss << " " << (j+1)
			<< " " << (ref_resource.capacity()*ref_resource.utilization_threshold())
			<< " " << vm_util_map.at(ptr_vm->id())
			<< " " << 0.2//FIXME: Minimum share is hard-coded
			<< ::std::endl;
	}
	oss << ";";// << ::std::endl;

	oss << "param wp := " << wp << ";" << ::std::endl;
	oss << "param ws := " << ws << ";" << ::std::endl;

	// Provide initial values (useful for local optimization techniques)
   if (!init_guess.empty())
	{
		typedef typename virtual_machines_placement<traits_type>::const_iterator vm_placement_iterator;
		typedef typename virtual_machines_placement<traits_type>::share_const_iterator vm_placement_share_iterator;

		::std::set< ::std::size_t > used_pms;
		oss << "var y :=" << ::std::endl;
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			oss << "[" << (i+1) << ",*]";
			for (::std::size_t j = 0; j < n_vms; ++j)
			{
				oss << " " << (j+1) << " ";
				if (init_guess.placed(vm_ids[j], pm_ids[i]))
				{
					oss << 1;
					used_pms.insert(i);
				}
				else
				{
					oss << 0;
				}
			}
			oss << ::std::endl;
		}
		oss << ";" << ::std::endl;

		oss << "var x :=" << ::std::endl;
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			oss << (i+1) << " " << (used_pms.count(i) > 0 ? 1 : 0) << ::std::endl;
		}
		oss << ";" << ::std::endl;

		oss << "var s :=" << ::std::endl;
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			oss << "[" << (i+1) << ",*]";
			for (::std::size_t j = 0; j < n_vms; ++j)
			{
				oss << " " << (j+1) << " ";
				if (init_guess.placed(vm_ids[j], pm_ids[i]))
				{
					vm_placement_iterator vmp_it(init_guess.find(vm_ids[j]));
					vm_placement_share_iterator share_end_it(init_guess.shares_end(vmp_it));
					bool found(false);
					vm_placement_share_iterator end_share_it(init_guess.shares_end(vmp_it));
					for (vm_placement_share_iterator share_it = init_guess.shares_begin(vmp_it); share_it != end_share_it; ++share_it)
					{
						//FIXME: CPU resource category is hard-coded
						if (init_guess.resource_category(share_it) == ::dcs::eesim::cpu_resource_category)
						{
							oss << init_guess.resource_share(share_it);
							found = true;
						}
					}

					if (!found)
					{
						throw ::std::runtime_error("[dcs::eesim::detail::gams::make_initial_vm_placement_problem] Incompatible resource categories.");
					}
				}
				else
				{
					oss << 0;
				}
			}
			oss << ::std::endl;
		}
		oss << ";" << ::std::endl;
	}

		return oss.str();
}


template <typename TraitsT>
inline
::std::string make_initial_vm_placement_problem_data(data_center<TraitsT> const& dc,
													 typename TraitsT::real_type wp,
													 typename TraitsT::real_type ws,
													 typename TraitsT::real_type ref_penalty,
													 ::std::map<typename TraitsT::virtual_machine_identifier_type,
															    typename TraitsT::real_type> const& vm_util_map,
													 ::std::vector<typename TraitsT::physical_machine_identifier_type>& pm_ids,
													 ::std::vector<typename TraitsT::virtual_machine_identifier_type>& vm_ids)
{
	return make_initial_vm_placement_problem_data(dc,
												  wp,
												  ws,
												  ref_penalty,
												  vm_util_map,
												  virtual_machines_placement<TraitsT>(),
												  pm_ids,
												  vm_ids);
}


inline
::std::string make_vm_placement_problem_model()
{
	::std::ostringstream oss;

//	oss << "reset;"
//		<< "model;"
	oss	<< "param ni > 0 integer;"
		<< "param nj > 0 integer;"
		<< "set I := 1..ni;"
		<< "set J := 1..nj;"
		<< "param c0{I};"
		<< "param c1{I};"
		<< "param c2{I};"
		<< "param r{I};"
		<< "param mc{I,J} >= 0;"
		//<< "param ur{J} >= 0, <= 1;"
		<< "param ur{J} >= 0;"
		<< "param Cr{J} >= 0;"
		//<< "param Smax{I} >= 0, <= 1;"
		<< "param Smax{I} >= 0;"
		//<< "param Srmin{J} >= 0, <= 1;"
		<< "param Srmin{J} >= 0;"
		<< "param C{I} >= 0;"
		<< "param Umax{I} >= 0;"
		<< "param wp >= 0 default 0;"
		<< "param wm >= 0 default 0;"
		<< "param ws >= 0 default 0;"
		<< "param eps := 1.0e-5;"
		<< "param wwp := wp / (ni * max{i in I} (c0[i]+c1[i]+c2[i]));"
		<< "param wwm := if wm > 0 then wm / (nj * max{i in I, j in J} mc[i,j]) else 0;"
		<< "param wws := ws / nj;"
		<< "param shares_sum;"
		<< "var x{I} binary;"
		<< "var y{I,J} binary;"
		<< "var s{I,J} >= 0, <= 1;"
		// NOTE: we use term "c2[i]*(eps+sum...) instead of "c2[i]*sum..." since
		// AMPL claims that the value of pow(0,0.4001) cannot be evaluated.
//		<< "minimize cost: wwp * sum{i in I} x[i]*(c0[i] + c1[i]*(sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps))) + c2[i]*(sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps)))^r[i]) + wwm * sum{i in I, j in J} mc[i,j]*y[i,j] + wws * sum{i in I, j in J} ((s[i,j]*C[i]/Cr[j]-1)^2)*y[i,j];"
		//<< "minimize cost: wwp * sum{i in I} x[i]*(c0[i] + c1[i]*(sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps))) + c2[i]*(sum{j in J} (y[i,j]+eps)*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps)))^r[i]) + wwm * sum{i in I, j in J} mc[i,j]*y[i,j] + wws * sum{i in I, j in J} ((s[i,j]*C[i]/Cr[j]-1)^2)*y[i,j];"
		//<< "minimize cost: wwp * sum{i in I} x[i]*(c0[i] + c1[i]*(sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps))) + c2[i]*(eps+sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps)))^r[i]) + wwm * sum{i in I, j in J} mc[i,j]*y[i,j] + wws * sum{i in I, j in J} ((s[i,j]*C[i]/Cr[j]-1)^2)*y[i,j];"
		<< "minimize cost: wwp * sum{i in I} x[i]*(c0[i] + c1[i]*(sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i])) + c2[i]*(eps+sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]))^r[i]) + wwm * sum{i in I, j in J} mc[i,j]*y[i,j] + wws * sum{i in I, j in J} ((s[i,j]*C[i]/Cr[j]-1)^2)*y[i,j];"
		<< "subject to one_vm_per_mach{j in J}: sum{i in I} y[i,j] = 1;"
		<< "subject to vm_on_active_mach1{i in I, j in J}: y[i,j] <= x[i];"
		<< "subject to vm_on_active_mach2{i in I}: sum{j in J} y[i,j] >= x[i];"
		<< "subject to valid_vm_share{i in I, j in J}: s[i,j] <= y[i,j];"
		<< "subject to min_vm_share{i in I, j in J}: s[i,j] >= y[i,j]*Srmin[j]*Cr[j]/C[i];"
		<< "subject to max_aggr_vm_share{i in I}: sum{j in J} s[i,j] <= Smax[i];"
		//<< "subject to valid_util{i in I}: sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]*(s[i,j]+eps)) <= Umax[i];"
		<< "subject to valid_util{i in I}: sum{j in J} y[i,j]*ur[j]*Cr[j]/(C[i]) <= Umax[i];"
		<< ::std::endl;

	return oss.str();
}


template <typename TraitsT, typename UtilFwdIterT, typename ShareFwdIterT>
::std::string make_vm_placement_problem_data(data_center<TraitsT> const& dc,
											 typename TraitsT::real_type wp,
											 typename TraitsT::real_type wm,
											 typename TraitsT::real_type ws,
//											 ::std::map<typename TraitsT::virtual_machine_identifier_type,
//													    typename TraitsT::real_type> const& vm_util_map,
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
	typedef ::std::map<vm_identifier_type, real_type> vm_utilization_container;
	typedef ::std::map<physical_resource_category, real_type> share_container;
	typedef ::std::map<vm_identifier_type, share_container> vm_share_container;


	::std::ostringstream oss;

	// Create the set of all physical machines
	pm_container pms(dc.physical_machines());

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

	::std::size_t n_pms(pms.size());

	// # of machines
	oss << "param ni := " << n_pms << ";" << ::std::endl;

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

	::std::size_t n_vms(active_vms.size());

	pm_ids = pm_identifier_container(n_pms);
	vm_ids = vm_identifier_container(n_vms);

	// # of VMs
	oss << "param nj := " << n_vms << ";" << ::std::endl;

	// Power model coefficients, max share, resource capacity, and utilization threshold
	oss << "param: c0 c1 c2 r Smax C Umax := " << ::std::endl;
	for (::std::size_t i = 0; i < n_pms; ++i)
	{
		pm_pointer ptr_pm(pms[i]);

		pm_ids[i] = ptr_pm->id();

		//FIXME: CPU resource category is hard-coded
		resource_pointer ptr_resource(ptr_pm->resource(::dcs::eesim::cpu_resource_category));
		energy_model_type const& energy_model(ptr_resource->energy_model());
		//FIXME: Fan2007 energy model type is hard-coded
		fan2007_energy_model_impl_type const* ptr_energy_model_impl(dynamic_cast<fan2007_energy_model_impl_type const*>(&energy_model));
		if (!ptr_energy_model_impl)
		{
			throw ::std::runtime_error("[dcs::eesim::detail::ampl::detail::make_vm_placement_problem_data] Unable to retrieve energy model.");
		}

		oss << (i+1)
			<< " " << ptr_energy_model_impl->coefficient(0) // c0
			<< " " << ptr_energy_model_impl->coefficient(1) // c1
			<< " " << ptr_energy_model_impl->coefficient(2) // c2
			<< " " << ptr_energy_model_impl->coefficient(3) // r
			<< " " << 1 // Smax
			<< " " << (ptr_resource->capacity()*ptr_resource->utilization_threshold()) // C
			<< " " << ptr_resource->utilization_threshold() // Umax
			<< ::std::endl;
	}
	oss << ";" << ::std::endl;

	// Migration costs
	oss << "param mc := " << ::std::endl;
	for (::std::size_t i = 0; i < n_pms; ++i)
	{
		pm_pointer ptr_pm(pms[i]);

		oss << "[" << (i+1) << ",*]";
		for (::std::size_t j = 0; j < n_vms; ++j)
		{
			vm_pointer ptr_vm(active_vms[j]);

			real_type cost(0);

			if (ptr_pm->id() != ptr_vm->vmm().hosting_machine().id())
			{
				cost = 1;
			}

			oss << " " << (j+1) << " " << cost;
		}
		oss << ::std::endl;
	}
	oss << ";";// << ::std::endl;

	// Reference machine capacity, utilization and min share of tiers
	oss << "param: Cr ur Srmin := " << ::std::endl;
	for (::std::size_t j = 0; j < n_vms; ++j)
	{
		vm_pointer ptr_vm(active_vms[j]);

		vm_ids[j] = ptr_vm->id();

		application_type const& app(ptr_vm->guest_system().application());

		//FIXME: CPU resource category is hard-coded
		reference_resource_type const& ref_resource(app.reference_resource(::dcs::eesim::cpu_resource_category));

		oss << " " << (j+1)
			<< " " << (ref_resource.capacity()*ref_resource.utilization_threshold())
//				<< " " << app.performance_model().tier_measure(ptr_vm->guest_system().id(), ::dcs::eesim::utilization_performance_measure)
			<< " " << vm_util_map.at(ptr_vm->id())
			<< " " << 0.2 //FIXME: Minimum share is hard-coded
			<< ::std::endl;
	}
	oss << ";" << ::std::endl;

	oss << "param wp := " << wp << ";" << ::std::endl;
	oss << "param wm := " << wm << ";" << ::std::endl;
	oss << "param ws := " << ws << ";" << ::std::endl;

	// Provide initial values (useful for local optimization techniques)
   if (!init_guess.empty())
	{
		typedef typename virtual_machines_placement<traits_type>::const_iterator vm_placement_iterator;
		typedef typename virtual_machines_placement<traits_type>::share_const_iterator vm_placement_share_iterator;

		::std::set< ::std::size_t > used_pms;
		oss << "var y :=" << ::std::endl;
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			oss << "[" << (i+1) << ",*]";
			for (::std::size_t j = 0; j < n_vms; ++j)
			{
				oss << " " << (j+1) << " ";
				if (init_guess.placed(vm_ids[j], pm_ids[i]))
				{
					oss << 1;
					used_pms.insert(i);
				}
				else
				{
					oss << 0;
				}
			}
			oss << ::std::endl;
		}
		oss << ";" << ::std::endl;

		oss << "var x :=" << ::std::endl;
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			oss << (i+1) << " " << (used_pms.count(i) > 0 ? 1 : 0) << ::std::endl;
		}
		oss << ";" << ::std::endl;

		oss << "var s :=" << ::std::endl;
		for (::std::size_t i = 0; i < n_pms; ++i)
		{
			oss << "[" << (i+1) << ",*]";
			for (::std::size_t j = 0; j < n_vms; ++j)
			{
				oss << " " << (j+1) << " ";
				if (init_guess.placed(vm_ids[j], pm_ids[i]))
				{
					vm_placement_iterator vmp_it(init_guess.find(vm_ids[j]));
					vm_placement_share_iterator share_end_it(init_guess.shares_end(vmp_it));
					bool found(false);
					vm_placement_share_iterator end_share_it(init_guess.shares_end(vmp_it));
					for (vm_placement_share_iterator share_it = init_guess.shares_begin(vmp_it); share_it != end_share_it; ++share_it)
					{
						//FIXME: CPU resource category is hard-coded
						if (init_guess.resource_category(share_it) == ::dcs::eesim::cpu_resource_category)
						{
							oss << init_guess.resource_share(share_it);
							found = true;
						}
					}

					if (!found)
					{
						throw ::std::runtime_error("[dcs::eesim::detail::gams::make_initial_vm_placement_problem] Incompatible resource categories.");
					}
				}
				else
				{
					oss << 0;
				}
			}
			oss << ::std::endl;
		}
		oss << ";" << ::std::endl;
	}

	return oss.str();
}


template <typename TraitsT, typename UtilFwdIterT, typename ShareFwdIterT>
inline
::std::string make_vm_placement_problem_data(data_center<TraitsT> const& dc,
											 typename TraitsT::real_type wp,
											 typename TraitsT::real_type wm,
											 typename TraitsT::real_type ws,
//											 ::std::map<typename TraitsT::virtual_machine_identifier_type,
//													    typename TraitsT::real_type> const& vm_util_map,
											 UtilFwdIterT vm_util_first,
											 UtilFwdIterT vm_util_last,
											 ShareFwdIterT vm_share_first,
											 ShareFwdIterT vm_share_last,
											 ::std::vector<typename TraitsT::physical_machine_identifier_type>& pm_ids,
											 ::std::vector<typename TraitsT::virtual_machine_identifier_type>& vm_ids)
{
	return make_vm_placement_problem_data(dc,
										  wp,
										  wm,
										  ws,
										  vm_util_first,
										  vm_util_last,
										  vm_share_first,
										  vm_share_last,
										  virtual_machines_placement<TraitsT>(),
										  pm_ids,
										  vm_ids);
}

}} // Namespace detail::<unnamed>


template <typename TraitsT>
struct vm_placement_problem
{
	typedef TraitsT traits_type;
	typedef ::std::vector<typename traits_type::physical_machine_identifier_type> physical_machine_identifier_container;
	typedef ::std::vector<typename traits_type::virtual_machine_identifier_type> virtual_machine_identifier_container;

	//::std::string text;
	::std::string model;
	::std::string data;
    physical_machine_identifier_container pm_ids;
    virtual_machine_identifier_container vm_ids;
};


template <typename TraitsT>
inline
::std::string make_initial_vm_placement_problem_model(data_center<TraitsT> const& dc,
													  typename TraitsT::real_type wp,
													  typename TraitsT::real_type ws,
													  typename TraitsT::real_type ref_penalty,
													  ::std::map<typename TraitsT::virtual_machine_identifier_type,
															     typename TraitsT::real_type> const& vm_util_map)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(dc);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(wp);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ws);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ref_penalty);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(vm_util_map);

	return detail::make_initial_vm_placement_problem_model();
}


template <typename TraitsT>
inline
::std::string make_initial_vm_placement_problem_model(data_center<TraitsT> const& dc,
													  typename TraitsT::real_type wp,
													  typename TraitsT::real_type ws,
													  typename TraitsT::real_type ref_penalty,
													  ::std::map<typename TraitsT::virtual_machine_identifier_type,
															     typename TraitsT::real_type> const& vm_util_map,
													  virtual_machines_placement<TraitsT> const& init_guess)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(dc);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(wp);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ws);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ref_penalty);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(vm_util_map);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(init_guess);

	return detail::make_initial_vm_placement_problem_model();
}


template <typename TraitsT>
inline
::std::string make_initial_vm_placement_problem_data(data_center<TraitsT> const& dc,
													 typename TraitsT::real_type wp,
													 typename TraitsT::real_type ws,
													 typename TraitsT::real_type ref_penalty,
													 ::std::map<typename TraitsT::virtual_machine_identifier_type,
															    typename TraitsT::real_type> const& vm_util_map)
{
	typename vm_placement_problem<TraitsT>::physical_machine_identifier_container tmp_pm_ids;
	typename vm_placement_problem<TraitsT>::virtual_machine_identifier_container tmp_vm_ids;

	return detail::make_initial_vm_placement_problem_data(dc,
														  wp,
														  ws,
														  ref_penalty,
														  vm_util_map,
														  tmp_pm_ids,
														  tmp_vm_ids);
}


template <typename TraitsT>
inline
::std::string make_initial_vm_placement_problem_data(data_center<TraitsT> const& dc,
													 typename TraitsT::real_type wp,
													 typename TraitsT::real_type ws,
													 typename TraitsT::real_type ref_penalty,
													 ::std::map<typename TraitsT::virtual_machine_identifier_type,
															    typename TraitsT::real_type> const& vm_util_map,
													 virtual_machines_placement<TraitsT> const& init_guess)
{
	typename vm_placement_problem<TraitsT>::physical_machine_identifier_container tmp_pm_ids;
	typename vm_placement_problem<TraitsT>::virtual_machine_identifier_container tmp_vm_ids;

	return detail::make_initial_vm_placement_problem_data(dc,
														  wp,
														  ws,
														  ref_penalty,
														  vm_util_map,
														  init_guess,
														  tmp_pm_ids,
														  tmp_vm_ids);
}


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

	problem.model = detail::make_initial_vm_placement_problem_model();
	problem.data = detail::make_initial_vm_placement_problem_data(dc,
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

	problem.model = detail::make_initial_vm_placement_problem_model();
	problem.data = detail::make_initial_vm_placement_problem_data(dc,
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
::std::string make_vm_placement_problem_model(data_center<TraitsT> const& dc,
											  typename TraitsT::real_type wp,
											  typename TraitsT::real_type wm,
											  typename TraitsT::real_type ws,
											  UtilFwdIterT vm_util_first,
											  UtilFwdIterT vm_util_last,
											  ShareFwdIterT vm_share_first,
											  ShareFwdIterT vm_share_last)
//											  ::std::map<typename TraitsT::virtual_machine_identifier_type,
//													     typename TraitsT::real_type> const& vm_util_map)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(dc);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(wp);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(wm);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ws);
//	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(vm_util_map);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(vm_util_first);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(vm_util_last);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(vm_share_first);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(vm_share_last);

	return detail::make_vm_placement_problem_model();
}


template <typename TraitsT, typename UtilFwdIterT, typename ShareFwdIterT>
inline
::std::string make_vm_placement_problem_model(data_center<TraitsT> const& dc,
											  typename TraitsT::real_type wp,
											  typename TraitsT::real_type wm,
											  typename TraitsT::real_type ws,
//											  ::std::map<typename TraitsT::virtual_machine_identifier_type,
//													     typename TraitsT::real_type> const& vm_util_map,
											  UtilFwdIterT vm_util_first,
											  UtilFwdIterT vm_util_last,
											  ShareFwdIterT vm_share_first,
											  ShareFwdIterT vm_share_last,
											  virtual_machines_placement<TraitsT> const& init_guess)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(dc);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(wp);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(wm);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(ws);
//	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(vm_util_map);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(vm_util_first);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(vm_util_last);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(vm_share_first);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(vm_share_last);
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(init_guess);

	return detail::make_vm_placement_problem_model();
}


template <typename TraitsT, typename UtilFwdIterT, typename ShareFwdIterT>
inline
::std::string make_vm_placement_problem_data(data_center<TraitsT> const& dc,
											 typename TraitsT::real_type wp,
											 typename TraitsT::real_type wm,
											 typename TraitsT::real_type ws,
										     UtilFwdIterT vm_util_first,
										     UtilFwdIterT vm_util_last,
										     ShareFwdIterT vm_share_first,
										     ShareFwdIterT vm_share_last)
//											 ::std::map<typename TraitsT::virtual_machine_identifier_type,
//													    typename TraitsT::real_type> const& vm_util_map)
{
	typename vm_placement_problem<TraitsT>::physical_machine_identifier_container tmp_pm_ids;
	typename vm_placement_problem<TraitsT>::virtual_machine_identifier_container tmp_vm_ids;

	return detail::make_vm_placement_problem_data(dc,
												  wp,
												  wm,
												  ws,
												  vm_util_first,
												  vm_util_last,
												  vm_share_first,
												  vm_share_last,
												  tmp_pm_ids,
												  tmp_vm_ids);
}


template <typename TraitsT, typename UtilFwdIterT, typename ShareFwdIterT>
inline
::std::string make_vm_placement_problem_data(data_center<TraitsT> const& dc,
											 typename TraitsT::real_type wp,
											 typename TraitsT::real_type wm,
											 typename TraitsT::real_type ws,
//											 ::std::map<typename TraitsT::virtual_machine_identifier_type,
//													    typename TraitsT::real_type> const& vm_util_map,
											 UtilFwdIterT vm_util_first,
											 UtilFwdIterT vm_util_last,
											 ShareFwdIterT vm_share_first,
											 ShareFwdIterT vm_share_last,
											 virtual_machines_placement<TraitsT> const& init_guess)
{
	typename vm_placement_problem<TraitsT>::physical_machine_identifier_container tmp_pm_ids;
	typename vm_placement_problem<TraitsT>::virtual_machine_identifier_container tmp_vm_ids;

	return detail::make_vm_placement_problem_data(dc,
												  wp,
												  wm,
												  ws,
												  vm_util_first,
												  vm_util_last,
												  vm_share_first,
												  vm_share_last,
												  init_guess,
												  tmp_pm_ids,
												  tmp_vm_ids);
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
//																   typename TraitsT::real_type> const& vm_util_map)
{
	vm_placement_problem<TraitsT> problem;

	problem.model = detail::make_vm_placement_problem_model();
	problem.data = detail::make_vm_placement_problem_data(dc,
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
//														::std::map<typename TraitsT::virtual_machine_identifier_type,
//																   typename TraitsT::real_type> const& vm_util_map,
														UtilFwdIterT vm_util_first,
														UtilFwdIterT vm_util_last,
														ShareFwdIterT vm_share_first,
														ShareFwdIterT vm_share_last,
														virtual_machines_placement<TraitsT> const& init_guess)
{
	vm_placement_problem<TraitsT> problem;

	problem.model = detail::make_vm_placement_problem_model();
	problem.data = detail::make_vm_placement_problem_data(dc,
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

}}}} // Namespace dcs::eesim::detail::ampl


#endif // DCS_EESIM_DETAIL_AMPL_VM_PLACEMENT_PROBLEM_HPP
