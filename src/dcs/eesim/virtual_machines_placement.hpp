#ifndef DCS_EESIM_VIRTUAL_MACHINES_PLACEMENT_HPP
#define DCS_EESIM_VIRTUAL_MACHINES_PLACEMENT_HPP


#include <cstddef>
#include <dcs/eesim/physical_resource_category.hpp>
# include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>


namespace dcs { namespace eesim {

template <typename TraitsT>
class virtual_machines_placement
{
	/*
	 * Internal storage structure:
	 *   placement_container: it is a map where each entry is identified by a pair of virtual-machine and physical-machine identifier and contains the a collection of resouce-category to share mappings. 
	 *     [
	 *       <virtual-machine-id, physical-machine-id> => [ resource-category => share, ... ],
	 *       ...
	 *     ]
	 *   by_vm_index_container: it is a vector whose i-th element contains the physical machine identifier for the virtual machine identified by the number i.
	 *     [ physical-machine-id, ... ]
	 *   by_pm_index_container: it is a vector whose i-th element contains the set of virtual machine identifiers for the physical machine identified by the number i.
	 *     [ [ virtual-machine-id, ... ], [ virtual-machine-id, ... ] ]
	 */

	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef physical_machine<traits_type> physical_machine_type;
	public: typedef virtual_machine<traits_type> virtual_machine_type;
	private: typedef typename traits_type::virtual_machine_identifier_type physical_machine_identifier_type;
	private: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
	private: typedef ::std::pair<virtual_machine_identifier_type,physical_machine_identifier_type> vm_pm_pair_type;
	private: typedef ::std::map<physical_resource_category,real_type> share_container;
	private: typedef ::std::map<vm_pm_pair_type,share_container> placement_container;
	//private: typedef ::std::vector<physical_machine_identifier_type> by_vm_index_container;
	private: typedef ::std::map<physical_machine_identifier_type,virtual_machine_identifier_type> by_vm_index_container;
	private: typedef ::std::set<virtual_machine_identifier_type> by_pm_index_subcontainer;
	//private: typedef ::std::vector<by_pm_index_subcontainer> by_pm_index_container;
	private: typedef ::std::map<physical_machine_identifier_type,by_pm_index_subcontainer> by_pm_index_container;
	public: typedef typename placement_container::iterator iterator;
	public: typedef typename placement_container::const_iterator const_iterator;
	public: typedef typename share_container::iterator share_iterator;
	public: typedef typename share_container::const_iterator share_const_iterator;


//	public: virtual_machines_placement(::std::size_t nvms, ::std::size_t npms)
//	: placements_(),
//	  by_vm_idx_(nvms),
//	  by_pm_idx_(npms)
//	{
//	}


	public: virtual_machines_placement()
	{
	}


	public: template <typename ForwardIterT>
		bool placeable(virtual_machine_type& vm,
					   physical_machine_type& pm,
					   ForwardIterT first_share,
					   ForwardIterT last_share)
	{
		typedef typename share_container::const_iterator share_iterator;
		typedef typename by_pm_index_subcontainer::const_iterator pm_vm_iterator;

		physical_machine_identifier_type pm_id = pm.id();
		virtual_machine_identifier_type vm_id = vm.id();

DCS_DEBUG_TRACE("Is VM " << vm_id << " placeable into PM " << pm_id << "?");//XXX
DCS_DEBUG_TRACE("Wanted space");//XXX
for (ForwardIterT share_it = first_share; share_it != last_share; ++share_it)//XXX
{//XXX
	DCS_DEBUG_TRACE("Wanted space for resource " << share_it->first << ": " << share_it->second);//XXX
}//XXX
//		share_container wanted_shares(first_share, last_share);
		share_container free_shares;
		share_container wanted_shares;
//
		// Initialize wanted and free shares containers and make some
		// preliminary check.
		while (first_share != last_share)
		{
			physical_resource_category category = first_share->first;
			real_type max_share = pm.resource(category)->utilization_threshold();
			real_type wanted_share = first_share->second;
			if (wanted_share > max_share)
			{
DCS_DEBUG_TRACE("Lack of Free space for resource " << category << " -> Max Available: " << max_share << " - Requested: " << wanted_share);//XXX
DCS_DEBUG_TRACE("FALSE");//XXX
				return false;
			}
//
//			//wanted_shares.insert(::std::make_pair(category, wanted_share));
			wanted_shares[category] = wanted_share;
			free_shares[category] = max_share;
			++first_share;
		}

		pm_vm_iterator pm_vm_end_it = by_pm_idx_[pm_id].end();
		for (pm_vm_iterator pm_vm_it = by_pm_idx_[pm_id].begin(); pm_vm_it != pm_vm_end_it; ++pm_vm_it)
		{
			// Iterate over each VM deployed on this machine (but the one we
			// currently want to place)

			//vm_pm_pair_type key = ::std::make_pair(*pm_vm_it, pm_id);
			vm_pm_pair_type key = make_vm_pm_pair(*pm_vm_it, pm_id);

			if (*pm_vm_it != vm_id)
			{
				for (share_iterator share_it = placements_[key].begin(); share_it != placements_[key].end(); ++share_it)
				{
					physical_resource_category category = share_it->first;
//					real_type share = share_it->second;

					free_shares[category] -= share_it->second;
//					if (free_shares.count(category) > 0)
//					{
//						free_shares[category] -= share;
//					}
//					else
//					{
//						free_shares[category] = pm.resource(category)->utilization_threshold() - share;
//					}

					if ((wanted_shares.count(category) > 0 && free_shares[category] < wanted_shares[category]) || free_shares[category] <= 0)
					{
DCS_DEBUG_TRACE("Lack of Free space for resource " << category << " -> Available: " << free_shares[category] << " - Requested: " << wanted_shares[category]);//XXX
DCS_DEBUG_TRACE("FALSE");//XXX
						return false;
					}
				}
			}
		}

DCS_DEBUG_TRACE("Free space");//XXX
for (share_iterator share_it = free_shares.begin(); share_it != free_shares.end(); ++share_it)//XXX
{//XXX
	DCS_DEBUG_TRACE("Free space for resource " << share_it->first << "-> Before: " << share_it->second << " - After: " << (share_it->second-wanted_shares[share_it->first]));//XXX
}//XXX
DCS_DEBUG_TRACE("TRUE");//XXX
		return true;
	}


	public: template <typename ForwardIterT>
		bool try_place(virtual_machine_type& vm,
					   physical_machine_type& pm,
					   ForwardIterT first_share, // <category,share> pair
					   ForwardIterT last_share) // <category,share> pair
	{
		physical_machine_identifier_type pm_id = pm.id();
		virtual_machine_identifier_type vm_id = vm.id();

		if (!placeable(vm, pm, first_share, last_share))
		{
			return false;
		}

		//vm_pm_pair_type key = ::std::make_pair(vm_id, pm_id);
		vm_pm_pair_type key = make_vm_pm_pair(vm_id, pm_id);
		placements_[key] = share_container(first_share, last_share);
		while (first_share != last_share)
		{
			vm.resource_share(first_share->first, first_share->second);
			vm.wanted_resource_share(first_share->first, first_share->second);
			++first_share;
		}
		by_vm_idx_[vm_id] = pm_id;
		by_pm_idx_[pm_id].insert(vm_id);

		return true;
	}


	public: template <typename ForwardIterT>
		void place(virtual_machine_type const& vm,
				   physical_machine_type const& pm,
				   ForwardIterT first_share, // <category,share> pair
				   ForwardIterT last_share) // <category,share> pair
	{
		if (!try_place(vm, pm, first_share, last_share))
		{
			throw ::std::runtime_error("[dcs::eesim::virtual_machines_placement::place] Tried to place an unplaceable VM.");
		}
	}


	public: void displace(virtual_machine_type const& vm)
	{
//		// precondition: virtual machine pointer must be a valid pointer
//		DCS_ASSERT(
//			ptr_vm,
//			throw ::std::invalid_argument("[dcs::eesim::virtual_machines_placement::displace] Invalid virtual machine pointer.")
//		);
		// precondition: virtual machine must do exist
		DCS_ASSERT(
			by_vm_idx_.count(vm.id()) > 0,
			throw ::std::invalid_argument("[dcs::eesim::virtual_machines_placement::displace] Unknown virtual machine.")
		);

		virtual_machine_identifier_type vm_id = vm.id();
		physical_machine_identifier_type pm_id = by_vm_idx_[vm_id];

		//placements_.erase(::std::make_pair(vm_id, pm_id));
		placements_.erase(make_vm_pm_pair(vm_id, pm_id));
		by_vm_idx_.erase(vm_id);
		by_pm_idx_.erase(pm_id);
	}


	public: bool placed(virtual_machine_identifier_type vm_id)
	{
		return by_vm_idx_.count(vm_id) != 0;
	}


	public: void displace_all()
	{
		placements_.clear();
		by_vm_idx_.clear();
		by_pm_idx_.clear();
	}


	public: const_iterator begin() const
	{
		return placements_.begin();
	}


	public: iterator begin()
	{
		return placements_.begin();
	}


	public: const_iterator end() const
	{
		return placements_.end();
	}


	public: iterator end()
	{
		return placements_.end();
	}


	public: iterator find(virtual_machine_identifier_type const& vm)
	{
		//return placements_.find(::std::make_pair(vm_id, by_vm_idx_[vm_id]));
		return placements_.find(make_vm_pm_pair_by_vm(vm_id));
	}


	public: const_iterator find(virtual_machine_identifier_type const& vm) const
	{
		//return placements_.find(::std::make_pair(vm_id, by_vm_idx_[vm_id]));
		return placements_.find(make_vm_pm_pair_by_vm(vm_id));
	}


	public: iterator find(virtual_machine_type const& vm)
	{
		//return placements_.find(::std::make_pair(vm.id(), by_vm_idx_[vm.id()]));
		return placements_.find(make_vm_pm_pair_by_vm(vm.id()));
	}


	public: const_iterator find(virtual_machine_type const& vm) const
	{
		//return placements_.find(::std::make_pair(vm.id(), by_vm_idx_[vm.id()]));
		return placements_.find(make_vm_pm_pair_by_vm(vm.id()));
	}


	public: share_iterator shares_begin(iterator it)
	{
		return it->second.begin();
	}


	public: share_const_iterator shares_begin(const_iterator it) const
	{
		return it->second.begin();
	}


	public: share_iterator shares_end(iterator it)
	{
		return it->second.end();
	}


	public: share_const_iterator shares_end(const_iterator it) const
	{
		return it->second.end();
	}


	public: virtual_machine_identifier_type vm_id(vm_pm_pair_type const& pair) const
	{
		return pair.first;
	}


	public: virtual_machine_identifier_type vm_id(iterator it) const
	{
		return it->first.first;
	}


	public: virtual_machine_identifier_type vm_id(const_iterator it) const
	{
		return it->first.first;
	}


	public: physical_machine_identifier_type pm_id(vm_pm_pair_type const& pair) const
	{
		return pair.second;
	}


	public: physical_machine_identifier_type pm_id(iterator it) const
	{
		return it->first.second;
	}


	public: physical_machine_identifier_type pm_id(const_iterator it) const
	{
		return it->first.second;
	}


	public: physical_resource_category resource_category(share_iterator it) const
	{
		return it->first;
	}


	public: physical_resource_category resource_category(share_const_iterator it) const
	{
		return it->first;
	}


	public: real_type resource_share(share_iterator it) const
	{
		return it->second;
	}


	public: real_type resource_share(share_const_iterator it) const
	{
		return it->second;
	}


	private: vm_pm_pair_type make_vm_pm_pair(virtual_machine_identifier_type vm_id, physical_machine_identifier_type pm_id)
	{
		return ::std::make_pair(vm_id, pm_id);
	}


	private: vm_pm_pair_type make_vm_pm_pair_by_vm(virtual_machine_identifier_type vm_id)
	{
		return ::std::make_pair(vm_id, by_vm_idx_[vm_id]);
	}


	private: vm_pm_pair_type make_vm_pm_pair_by_pm(physical_machine_identifier_type pm_id)
	{
		return ::std::make_pair(by_pm_idx_[pm_id], pm_id);
	}


	template <
		typename CharT,
		typename CharTraitsT,
		typename ATraitsT
	>
	friend ::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, virtual_machines_placement<ATraitsT> const& placement);


	//private: matrix_type P_;
	private: placement_container placements_;
	private: by_vm_index_container by_vm_idx_;
	private: by_pm_index_container by_pm_idx_;
};


template <
	typename CharT,
	typename CharTraitsT,
	typename TraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, virtual_machines_placement<TraitsT> const& placement)
{
	typedef typename virtual_machines_placement<TraitsT>::placement_container placement_container;
	typedef typename virtual_machines_placement<TraitsT>::share_container share_container;
	typedef typename placement_container::const_iterator placement_iterator;
	typedef typename share_container::const_iterator share_iterator;

	placement_iterator place_begin_it = placement.placements_.begin();
	placement_iterator place_end_it = placement.placements_.end();
	for (placement_iterator place_it = place_begin_it; place_it != place_end_it; ++place_it)
	{
		if (place_it != place_begin_it)
		{
			os << ";";
		}
		os << "(VM: " << placement.vm_id(place_it->first) << ")->(PM: " << placement.pm_id(place_it->first) << "): <";
		share_iterator share_begin_it = place_it->second.begin();
		share_iterator share_end_it = place_it->second.end();
		for (share_iterator share_it = share_begin_it; share_it != share_end_it; ++share_it)
		{
			if (share_it != share_begin_it)
			{
				os << ",";
			}
			os << "(resource: " << share_it->first << ",share: " << share_it->second << ")";
		}
		os << ">";
	}

	return os;
}

}} // Namespace dcs::eesim


#endif // DCS_EESIM_VIRTUAL_MACHINES_PLACEMENT_HPP
