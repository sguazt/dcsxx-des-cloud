/**
 * \file dcs/des/cloud/base_incremental_placement_strategy.hpp
 *
 * \brief Base class for incremental VM placement strategies.
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

#ifndef DCS_DES_CLOUD_BASE_INCREMENTAL_PLACEMENT_STRATEGY_HPP
#define DCS_DES_CLOUD_BASE_INCREMENTAL_PLACEMENT_STRATEGY_HPP


#include <dcs/des/cloud/data_center.hpp>
#include <dcs/des/cloud/virtual_machines_placement.hpp>
#include <vector>


namespace dcs { namespace des { namespace cloud {

template <typename TraitsT>
class base_incremental_placement_strategy
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::virtual_machine_identifier_type virtual_machine_identifier_type;
	public: typedef data_center<traits_type> data_center_type;
	public: typedef virtual_machines_placement<traits_type> virtual_machines_placement_type;
	protected: typedef typename data_center_type::virtual_machine_pointer virtual_machine_pointer;
	protected: typedef ::std::vector<virtual_machine_pointer> virtual_machine_container;


	public: base_incremental_placement_strategy()
	: ref_penalty_(0)
	{
	}


	public: virtual ~base_incremental_placement_strategy() { }


//	public: virtual_machines_placement_type place(data_center_type const& dc, virtual_machine_container const& vms)
//	{
//		return do_place(dc, vms);
//	}


	public: template <typename ForwardIterT>
		virtual_machines_placement_type place(data_center_type const& dc, ForwardIterT first, ForwardIterT last)
	{
		return do_place(dc, virtual_machine_container(first, last));
	}


	public: void reference_share_penalty(real_type penalty)
	{
		ref_penalty_ = penalty;
	}


	public: real_type reference_share_penalty() const
	{
		return ref_penalty_;
	}


	private: virtual virtual_machines_placement_type do_place(data_center_type const& dc, virtual_machine_container const& vms) = 0;


	/// The penalty (in percentage) to assign to the reference share 
	private: real_type ref_penalty_;
};

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_BASE_INCREMENTAL_PLACEMENT_STRATEGY_HPP
