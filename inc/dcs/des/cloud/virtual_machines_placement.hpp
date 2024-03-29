/**
 * \file dcs/des/cloud/virtual_machines_placement.hpp
 *
 * \brief A placement of VMs.
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

#ifndef DCS_DES_CLOUD_VIRTUAL_MACHINES_PLACEMENT_HPP
#define DCS_DES_CLOUD_VIRTUAL_MACHINES_PLACEMENT_HPP


#include <cstddef>
#include <dcs/math/traits/float.hpp>
#include <dcs/des/cloud/physical_resource_category.hpp>
#include <dcs/des/cloud/utility.hpp>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>


namespace dcs { namespace des { namespace cloud {

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
	public: typedef data_center<traits_type> data_center_type;
	public: typedef typename data_center_type::physical_machine_type physical_machine_type;
	public: typedef typename data_center_type::virtual_machine_type virtual_machine_type;
	private: typedef typename traits_type::virtual_machine_identifier_type physical_machine_identifier_type;
	private: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
	private: typedef ::std::pair<virtual_machine_identifier_type,physical_machine_identifier_type> vm_pm_pair_type;
	private: typedef ::std::map<physical_resource_category,real_type> share_container;
	private: typedef ::std::map<vm_pm_pair_type,share_container> placement_container;
	//private: typedef ::std::vector<physical_machine_identifier_type> by_vm_index_container;
	private: typedef ::std::map<virtual_machine_identifier_type,physical_machine_identifier_type> by_vm_index_container;
	private: typedef ::std::set<virtual_machine_identifier_type> by_pm_index_subcontainer;
	//private: typedef ::std::vector<by_pm_index_subcontainer> by_pm_index_container;
	private: typedef ::std::map<physical_machine_identifier_type,by_pm_index_subcontainer> by_pm_index_container;
	public: typedef typename placement_container::size_type size_type;
	public: typedef typename placement_container::iterator iterator;
	public: typedef typename placement_container::const_iterator const_iterator;
	public: typedef typename share_container::iterator share_iterator;
	public: typedef typename share_container::const_iterator share_const_iterator;
	public: typedef ::std::map<physical_resource_category, real_type> resource_utilization_map;


	public: template <typename ForwardShareIterT, typename ForwardUtilIterT>
		bool placeable(virtual_machine_type const& vm,
					   physical_machine_type const& pm,
					   ForwardShareIterT first_share,
					   ForwardShareIterT last_share,
					   ForwardUtilIterT first_util,
					   ForwardUtilIterT last_util,
					   data_center_type const& dc)
	{
		typedef typename data_center_type::virtual_machine_pointer virtual_machine_pointer;
		typedef typename share_container::const_iterator share_iterator;
		typedef typename by_pm_index_subcontainer::const_iterator pm_vm_iterator;

		physical_machine_identifier_type pm_id(pm.id());
		virtual_machine_identifier_type vm_id(vm.id());

//		share_container wanted_shares(first_share, last_share);
		share_container free_shares;
		share_container wanted_shares;
		resource_utilization_map free_utils;
		resource_utilization_map wanted_utils;

		// Initialize wanted and free shares containers and make some
		// preliminary check.
		while (first_share != last_share)
		{
			physical_resource_category category(first_share->first);
			//real_type max_share(pm.resource(category)->utilization_threshold());
			real_type max_share(1);//FIXME: usare ref_penalty config parameter oppure prevedere un share_threshold per ogni risorsa fisica
			real_type wanted_share(first_share->second);
			if (::dcs::math::float_traits<real_type>::definitely_greater(wanted_share, max_share))
			{
				return false;
			}
			wanted_shares[category] = wanted_share;
			free_shares[category] = max_share;
//			if (!wanted_utils.empty())
//			{
//				free_utils[category] = pm.resource(category)->utilization_threshold();
//			}
			++first_share;
		}

		// Initialize wanted and free utilizations containers and make some
		// preliminary check.
		while (first_util != last_util)
		{
			physical_resource_category category(first_util->first);
			real_type max_util(pm.resource(category)->utilization_threshold());
			real_type wanted_util(first_util->second);
			if (::dcs::math::float_traits<real_type>::definitely_greater(wanted_util, max_util))
			{
				return false;
			}
			wanted_utils[category] = wanted_util;
			free_utils[category] = max_util;
			++first_util;
		}

		if (by_pm_idx_.count(pm_id) > 0)
		{
			pm_vm_iterator pm_vm_end_it(by_pm_idx_.at(pm_id).end());
			for (pm_vm_iterator pm_vm_it = by_pm_idx_.at(pm_id).begin(); pm_vm_it != pm_vm_end_it; ++pm_vm_it)
			{
				// Iterate over each VM deployed on this machine (but the one we
				// currently want to place)

				if (*pm_vm_it != vm_id)
				{
					vm_pm_pair_type key(make_vm_pm_pair(*pm_vm_it, pm_id));

					share_iterator share_end_it(placements_.at(key).end());
					for (share_iterator share_it = placements_.at(key).begin(); share_it != share_end_it; ++share_it)
					{
						physical_resource_category category(share_it->first);

						free_shares[category] -= share_it->second;

						if (
								(
									wanted_shares.count(category) > 0
									&&
									::dcs::math::float_traits<real_type>::definitely_less(free_shares.at(category), wanted_shares.at(category))
								)
								||
								free_shares[category] <= 0)
						{
							return false;
						}

						if (!wanted_utils.empty())
						{
							virtual_machine_pointer ptr_vm(dc.virtual_machine_ptr(*pm_vm_it));

							// check: paranoid check
							DCS_DEBUG_ASSERT( ptr_vm );

							free_utils[category] -= scale_resource_utilization(ptr_vm->guest_system().application().reference_resource(category).capacity(),
																			   ptr_vm->guest_system().resource_share(category),
																			   pm.resource(category)->capacity(),
																			   share_it->second,
																			   ptr_vm->guest_system().application().performance_model().tier_measure(ptr_vm->guest_system().id(), ::dcs::des::cloud::utilization_performance_measure),
																			   pm.resource(category)->utilization_threshold());
							if (
									(
										wanted_utils.count(category) > 0
										&&
										::dcs::math::float_traits<real_type>::definitely_less(free_utils.at(category), wanted_utils.at(category))
									)
									||
									free_utils[category] <= 0)
							{
								return false;
							}
						}
					}
				}
			}
		}

		return true;
	}


	public: template <typename ForwardIterT>
		bool placeable(virtual_machine_type const& vm,
					   physical_machine_type const& pm,
					   ForwardIterT first_share,
					   ForwardIterT last_share)
	{
		resource_utilization_map utils_map;
		return placeable(vm,
						 pm,
						 first_share,
						 last_share,
						 utils_map.begin(),
						 utils_map.end(),
						 data_center_type());
	}


	public: template <typename ForwardShareIterT, typename ForwardUtilIterT>
		bool try_place(virtual_machine_type& vm,
					   physical_machine_type& pm,
					   ForwardShareIterT first_share, // <category,share> pair
					   ForwardShareIterT last_share, // <category,share> pair
					   ForwardUtilIterT first_util, // <category,utilization> pair
					   ForwardUtilIterT last_util, // <category,utilization> pair
					   data_center_type const& dc)
	{
		physical_machine_identifier_type pm_id(pm.id());
		virtual_machine_identifier_type vm_id(vm.id());

		if (!placement_need_update(vm, pm, first_share, last_share))
		{
			return true;
		}

		if (!placeable(vm, pm, first_share, last_share, first_util, last_util, dc))
		{
			return false;
		}

		//vm_pm_pair_type key = ::std::make_pair(vm_id, pm_id);
		vm_pm_pair_type key(make_vm_pm_pair(vm_id, pm_id));
		placements_[key] = share_container(first_share, last_share);
//		while (first_share != last_share)
//		{
//			vm.wanted_resource_share(first_share->first, first_share->second);
//			vm.resource_share(first_share->first, first_share->second);
//			++first_share;
//		}
		by_vm_idx_[vm_id] = pm_id;
		by_pm_idx_[pm_id].insert(vm_id);

		return true;
	}


	public: template <typename ForwardIterT>
		bool try_place(virtual_machine_type& vm,
					   physical_machine_type& pm,
					   ForwardIterT first_share, // <category,share> pair
					   ForwardIterT last_share) // <category,share> pair
	{
		resource_utilization_map utils_map;
		return try_place(vm,
						 pm,
						 first_share,
						 last_share,
						 utils_map.begin(),
						 utils_map.end(),
						 data_center_type());
	}


	public: template <typename ForwardShareIterT, typename ForwardUtilIterT>
		void place(virtual_machine_type& vm,
				   physical_machine_type& pm,
				   ForwardShareIterT first_share, // <category,share> pair
				   ForwardShareIterT last_share, // <category,share> pair
				   ForwardUtilIterT first_util, // <category,utilization> pair
				   ForwardUtilIterT last_util, // <category,utilization> pair
				   data_center_type const& dc)
	{
		if (!try_place(vm, pm, first_share, last_share, first_util, last_util, dc))
		{
			throw ::std::runtime_error("[dcs::des::cloud::virtual_machines_placement::place] Tried to place an unplaceable VM.");
		}
	}


	public: template <typename ForwardIterT>
		void place(virtual_machine_type& vm,
				   physical_machine_type& pm,
				   ForwardIterT first_share, // <category,share> pair
				   ForwardIterT last_share) // <category,share> pair
	{
		resource_utilization_map utils_map;
		place(vm, pm,
			  first_share,
			  last_share,
			  utils_map.begin(),
			  utils_map.end(),
			  data_center_type());
	}


	public: template <typename ForwardShareIterT, typename ForwardUtilIterT>
		void replace(virtual_machine_type& vm,
					 physical_machine_type& pm,
					 ForwardShareIterT first_share, // <category,share> pair
					 ForwardShareIterT last_share, // <category,share> pair
					 ForwardUtilIterT first_util, // <category,utilization> pair
					 ForwardUtilIterT last_util, // <category,utilization> pair
					 data_center_type const& dc)
	{
		if (!placement_need_update(vm, pm, first_share, last_share))
		{
			return;
		}

		displace(vm);

		if (!try_place(vm, pm, first_share, last_share, first_util, last_util, dc))
		{
			throw ::std::runtime_error("[dcs::des::cloud::virtual_machines_placement::replace] Tried to place an unplaceable VM.");
		}
	}


	public: template <typename ForwardIterT>
		void replace(virtual_machine_type& vm,
					 physical_machine_type& pm,
					 ForwardIterT first_share, // <category,share> pair
					 ForwardIterT last_share) // <category,share> pair
	{
		resource_utilization_map utils_map;
		replace(vm, pm,
				first_share,
				last_share,
				utils_map.begin(),
				utils_map.end(),
				data_center_type());
	}


	public: void displace(virtual_machine_type const& vm)
	{
//		// precondition: virtual machine pointer must be a valid pointer
//		DCS_ASSERT(
//			ptr_vm,
//			throw ::std::invalid_argument("[dcs::des::cloud::virtual_machines_placement::displace] Invalid virtual machine pointer.")
//		);
		// precondition: virtual machine must do exist
		DCS_ASSERT(
			by_vm_idx_.count(vm.id()) > 0,
			throw ::std::invalid_argument("[dcs::des::cloud::virtual_machines_placement::displace] Unknown virtual machine.")
		);

		virtual_machine_identifier_type vm_id = vm.id();
		physical_machine_identifier_type pm_id = by_vm_idx_[vm_id];

		//placements_.erase(::std::make_pair(vm_id, pm_id));
		placements_.erase(make_vm_pm_pair(vm_id, pm_id));
		by_vm_idx_.erase(vm_id);
		by_pm_idx_[pm_id].erase(vm_id);
	}


	public: bool placed(virtual_machine_identifier_type vm_id) const
	{
		return by_vm_idx_.count(vm_id) != 0;
	}


	public: bool placed(virtual_machine_identifier_type vm_id,
						physical_machine_identifier_type pm_id) const
	{
		return placed(vm_id) && (by_vm_idx_.at(vm_id) == pm_id);
	}


	public: bool placed(virtual_machine_type const& vm) const
	{
		return placed(vm.id());
	}


	public: bool placed(virtual_machine_type const& vm,
						physical_machine_type const& pm) const
	{
		return placed(vm.id(), pm.id());
	}


	public: void displace_all()
	{
		placements_.clear();
		by_vm_idx_.clear();
		by_pm_idx_.clear();
	}


	public: size_type size() const
	{
		return placements_.size();
	}


	public: size_type pm_size() const
	{
		return by_pm_idx_.size();
	}


	public: size_type vm_size() const
	{
		return by_vm_idx_.size();
	}


	public: bool empty() const
	{
		return placements_.empty();
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


	public: iterator find(virtual_machine_identifier_type vm_id)
	{
		if (!placed(vm_id))
		{
			return placements_.end();
		}

		return placements_.find(make_vm_pm_pair_by_vm(vm_id));
	}


	public: const_iterator find(virtual_machine_identifier_type vm_id) const
	{
		if (!placed(vm_id))
		{
			return placements_.end();
		}

		return placements_.find(make_vm_pm_pair_by_vm(vm_id));
	}


	public: iterator find(virtual_machine_type const& vm)
	{
		if (!placed(vm))
		{
			return placements_.end();
		}

		return placements_.find(make_vm_pm_pair_by_vm(vm.id()));
	}


	public: const_iterator find(virtual_machine_type const& vm) const
	{
		if (!placed(vm))
		{
			return placements_.end();
		}

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


	private: vm_pm_pair_type make_vm_pm_pair(virtual_machine_identifier_type vm_id, physical_machine_identifier_type pm_id) const
	{
		return ::std::make_pair(vm_id, pm_id);
	}


	private: vm_pm_pair_type make_vm_pm_pair_by_vm(virtual_machine_identifier_type vm_id) const
	{
		return ::std::make_pair(vm_id, by_vm_idx_.at(vm_id));
	}


//[XXX] NO: there is zero or more VMs for a given PM
//	private: vm_pm_pair_type make_vm_pm_pair_by_pm(physical_machine_identifier_type pm_id) const
//	{
//		return ::std::make_pair(by_pm_idx_.at(pm_id), pm_id);
//	}
//[/XXX]


	private: template <typename ForwardShareIterT>
		bool placement_need_update(virtual_machine_type& vm,
								   physical_machine_type& pm,
								   ForwardShareIterT first_share, // <category,share> pair
								   ForwardShareIterT last_share) // <category,share> pair
	{
		if (!placed(vm, pm))
		{
			return true;
		}

		vm_pm_pair_type key(make_vm_pm_pair(vm.id(), pm.id()));

		for (ForwardShareIterT share_it = first_share; share_it != last_share; ++share_it)
		{
			physical_resource_category category(share_it->first);

			real_type old_share(placements_.at(key).at(category));
			real_type new_share(share_it->second);
			if (!::dcs::math::float_traits<real_type>::approximately_equal(old_share, new_share, 1.0e-5))
			{
				return true;
			}
		}

		return false;
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

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_VIRTUAL_MACHINES_PLACEMENT_HPP
