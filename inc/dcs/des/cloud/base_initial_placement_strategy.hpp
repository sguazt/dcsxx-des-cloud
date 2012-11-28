/**
 * \file dcs/des/cloud/base_initial_placement_strategy.hpp
 *
 * \brief Base class for initial VM placement strategies.
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

#ifndef DCS_DES_CLOUD_BASE_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_DES_CLOUD_BASE_INITIAL_PLACEMENT_STRATEGY_HPP


#include <dcs/des/cloud/data_center.hpp>
#include <dcs/des/cloud/virtual_machines_placement.hpp>


namespace dcs { namespace des { namespace cloud {

template <typename TraitsT>
class base_initial_placement_strategy
{
	public: typedef TraitsT traits_type;
	public: typedef typename traits_type::real_type real_type;


	public: static const real_type default_reference_share_penalty;


	public: explicit base_initial_placement_strategy(real_type ref_penalty = default_reference_share_penalty)
	: ref_penalty_(ref_penalty)
	{
	}


	public: virtual ~base_initial_placement_strategy() { }


	public: virtual_machines_placement<traits_type> placement(data_center<traits_type> const& dc)
	{
		return do_placement(dc);
	}


	public: void reference_share_penalty(real_type penalty)
	{
		ref_penalty_ = penalty;
	}


	public: real_type reference_share_penalty() const
	{
		return ref_penalty_;
	}


	private: virtual virtual_machines_placement<traits_type> do_placement(data_center<traits_type> const& dc) = 0;


	/// The penalty (in percentage) to assign to the reference share 
	private: real_type ref_penalty_;
}; // base_initial_placement_strategy

template <typename TraitsT>
const typename TraitsT::real_type base_initial_placement_strategy<TraitsT>::default_reference_share_penalty(0);

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_BASE_INITIAL_PLACEMENT_STRATEGY_HPP
