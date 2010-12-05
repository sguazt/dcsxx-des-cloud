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


	public: virtual ~base_initial_placement_strategy() { }


	public: virtual_machines_placement<traits_type> placement(data_center<traits_type> const& dc)
	{
		return do_placement(dc);
	}


	private: virtual virtual_machines_placement<traits_type> do_placement(data_center<traits_type> const& dc) = 0;
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_BASE_INITIAL_PLACEMENT_STRATEGY_HPP
