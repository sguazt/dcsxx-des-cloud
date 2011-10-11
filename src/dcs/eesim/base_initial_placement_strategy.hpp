#ifndef DCS_EESIM_BASE_INITIAL_PLACEMENT_STRATEGY_HPP
#define DCS_EESIM_BASE_INITIAL_PLACEMENT_STRATEGY_HPP


#include <dcs/eesim/data_center.hpp>
#include <dcs/eesim/virtual_machines_placement.hpp>


namespace dcs { namespace eesim {

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

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_INITIAL_PLACEMENT_STRATEGY_HPP
