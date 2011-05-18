#ifndef DCS_EESIM_FIRST_FIT_SCALEOUT_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_EESIM_FIRST_FIT_SCALEOUT_INITIAL_PLACEMENT_STRATEGY_HPP


#include <cstddef>
#include <dcs/eesim/base_initial_placement_strategy.hpp>
#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/registry.hpp>
#include <dcs/eesim/physical_machine.hpp>
#include <dcs/eesim/physical_resource_category.hpp>
#include <dcs/eesim/virtual_machine.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>
#include <dcs/math/stats/distribution/discrete_uniform.hpp>
#include <dcs/math/stats/function/rand.hpp>
#include <dcs/memory.hpp>
#include <set>
#include <stdexcept>
#include <vector>


namespace dcs { namespace eesim {

namespace detail { namespace /*<unnamed>*/ {

// At beginning of step i the array p[0.. i-1] contains a permutation of the
// numbers 0.. i-1.
// In order to construct in the array p[0..i] a permutation of 0..i, choose a
// random location to put i and save the old value in that location at p[i].
/*
template <typename IntT, typename RandomAccessIterT, typename UniformRandomGeneratorT>
RandomAccessIterT rand_permutation(RandomAccessIterT first, RanomdAccessIterT last, UniformRandomGeneratorT& gen)
{
	IntT i = 0;
	RandomAccessIterT ret = first;
	while (first != last)
	{
		IntT j = gen() % (i+1);
		*(first+i) = *(first+j);
		*(first+j) = i;
		++i
		++first
	}

	return ret;
}

template <typename IntT, typename UniformRandomGeneratorT>
IntT* rand_permutation(IntT* p, IntT n, UniformRandomGeneratorT& gen)
{
	for (IntT i = 0; i < n; ++i)
	{
		IntT j = gen() % (i + 1);
		p[i] = p[j];
		p[j] = i;
	}

	return p;
}

template <typename IntT, typename UniformRandomGeneratorT>
::std::vector<IntT> rand_permutation(::std::vector<IntT> p, UniformRandomGeneratorT& gen)
{
	typename ::std::vector<IntT>::size_type n = p.size();

	for (IntT i = 0; i < n; ++i)
	{
		IntT j = gen() % (i + 1);
		p[i] = p[j];
		p[j] = i;
	}

	return p;
}

template <typename IntT, typename UniformRandomGeneratorT>
struct random_compare
{
	random_compare(UniformGeneratorT& gen) : gen_(gen) { }
	bool operator()(IntT a, IntT b) { return gen_() % 2; }
	UniformGeneratorT& gen_;
};

template <typename IntT, typename UniformRandomGeneratorT>
void random_permutation(RandomAccessIterT first, RandomAccessIterT last, UniformRandomGeneratorT& gen)
{
	sort(first, last, random_compare<IntT,UniformRandomGeneratorT>(gen));
//	return first;
}
*/

}} // Namespace detail::<unnamed>


/// Assign at most one virtual machine to each physical machine.
template <typename TraitsT>
class first_fit_scaleout_initial_placement_strategy: public base_initial_placement_strategy<TraitsT>
{
	private: typedef base_initial_placement_strategy<TraitsT> base_type;
	public: typedef TraitsT traits_type;
	private: typedef typename traits_type::real_type real_type;


	private: virtual_machines_placement<traits_type> do_placement(data_center<traits_type> const& dc)
	{
		typedef physical_machine<traits_type> physical_machine_type;
		typedef virtual_machine<traits_type> virtual_machine_type;
		typedef ::dcs::shared_ptr<physical_machine_type> physical_machine_pointer;
		typedef ::dcs::shared_ptr<virtual_machine_type> virtual_machine_pointer;
		typedef typename physical_machine_type::identifier_type pm_identifier_type;
		typedef typename virtual_machine_type::identifier_type vm_identifier_type;
		typedef ::std::vector<physical_machine_pointer> pm_container;
		typedef ::std::vector<virtual_machine_pointer> vm_container;
		typedef typename pm_container::const_iterator pm_iterator;
		typedef typename vm_container::const_iterator vm_iterator;
		typedef ::std::pair<physical_resource_category,real_type> share_type;
        typedef ::std::vector<share_type> share_container;
        typedef typename share_container::const_iterator share_iterator;
		typedef ::std::size_t size_type;
		typedef multi_tier_application<traits_type> application_type;

		vm_container vms(dc.virtual_machines());

		pm_container machs(dc.physical_machines());

		size_type nmachs = machs.size();
		size_type nvms = vms.size();

DCS_DEBUG_TRACE("BEGIN Initial Placement");//XXX
DCS_DEBUG_TRACE("#Machines: " << nmachs);//XXX
DCS_DEBUG_TRACE("#VMs: " << nvms);//XXX

		if (nvms > nmachs)
		{
			throw ::std::logic_error("[dcs::eesim::first_fit_scaleout_initial_placement_strategy] Too many virtual machines.");
		}

		virtual_machines_placement<traits_type> deployment;

		::std::vector< ::std::pair<physical_resource_category,real_type> > shares;
		::std::set<pm_identifier_type> used_machs;
		vm_iterator vm_end_it = vms.end();
		for (vm_iterator vm_it = vms.begin(); vm_it != vm_end_it; ++vm_it)
		{
			virtual_machine_pointer ptr_vm(*vm_it);
			physical_machine_pointer ptr_mach;
			application_type const& app(ptr_vm->guest_system().application());
			bool placed(false);

			// Retrieve the share for every resource of the VM guest system
			share_container ref_shares(ptr_vm->guest_system().resource_shares());
 
			// For each physical machine PM, try to deploy current VM on PM
			// until a suitable machine (i.e., a machine with sufficient free
			// capacity) is found.
			pm_iterator pm_end_it = machs.end();
			for (pm_iterator pm_it = machs.begin(); pm_it != pm_end_it && !placed; ++pm_it)
			{
				ptr_mach = *pm_it;

				if (used_machs.count(ptr_mach->id()))
				{
					// Machine already occupied
					continue;
				}

				// Reference to actual resource shares
				share_container shares;
				share_iterator ref_share_end_it = ref_shares.end();
				for (share_iterator ref_share_it = ref_shares.begin(); ref_share_it != ref_share_end_it; ++ref_share_it)
				{
					physical_resource_category ref_category(ref_share_it->first);
					real_type ref_share(ref_share_it->second);
					if (this->reference_share_penalty() > 0)
					{
						ref_share -= ref_share*this->reference_share_penalty();
					}
					real_type ref_capacity(app.reference_resource(ref_category).capacity());
					real_type ref_threshold(app.reference_resource(ref_category).utilization_threshold());

					real_type actual_capacity(ptr_mach->resource(ref_category)->capacity());
					real_type actual_threshold(ptr_mach->resource(ref_category)->utilization_threshold());

					real_type share;
					share = ::dcs::eesim::scale_resource_share(ref_capacity,
															   ref_threshold,
															   actual_capacity,
															   actual_threshold,
															   ref_share);

					shares.push_back(share_type(ref_category, share));
				}


                placed = deployment.try_place(*ptr_vm,
                                              *ptr_mach,
                                              shares.begin(),
                                              shares.end());
			}
DCS_DEBUG_TRACE("Placed: VM(" << ptr_vm->id() << ") -> PM(" << ptr_mach->id() << ") ==> OK? " <<  std::boolalpha << placed);///XXX

			if (placed)
			{
//				machs.erase(ptr_mach);
				used_machs.insert(ptr_mach->id());
			}
		}

DCS_DEBUG_TRACE("END Initial Placement ==> " << deployment);///XXX
		return deployment;
	}
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_FIRST_FIT_SCALEOUT_INITIAL_PLACEMENT_STRATEGY_HPP
