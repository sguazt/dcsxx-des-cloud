/**
 * \file dcs/eesim/application_performance_model_traits.hpp
 *
 * \brief Traits class for application performance model.
 *
 * Copyright (C) 2009-2011  Distributed Computing System (DCS) Group, Computer
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

#ifndef DCS_EESIM_APPLICATION_PERFORMANCE_MODEL_TRAITS_HPP
#define DCS_EESIM_APPLICATION_PERFORMANCE_MODEL_TRAITS_HPP


#include <dcs/eesim/performance_measure_category.hpp>
//#include <dcs/perfeval/qn/open_multi_bcmp_network.hpp>
//#include <limits>


namespace dcs { namespace eesim {

template <typename TraitsT, typename ModelT>
class application_performance_model_traits
{
	public: typedef TraitsT traits_type;
	public: typedef ModelT model_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;


	public: static real_type application_measure(model_type const& model, performance_measure_category category);

	public: static real_type tier_measure(model_type const& model, uint_type tier_id, performance_measure_category category);
};


/*XXX: moved to src/application_performance_model_traits.hpp
 
template <typename TraitsT, typename RealT, typename UIntT>
class application_performance_model_traits<
			TraitsT,
			::dcs::perfeval::qn::open_multi_bcmp_network<RealT,UIntT>
		>
{
	public: typedef TraitsT traits_type;
	public: typedef dcs::perfeval::qn::open_multi_bcmp_network<RealT,UIntT> model_type;
	public: typedef typename traits_type::real_type real_type;
	public: typedef typename traits_type::uint_type uint_type;


	public: static real_type application_measure(model_type const& model, performance_measure_category category)
	{
		switch (category)
		{
			case response_time_performance_measure:
				return model.system_response_time();
			default:
				return ::std::numeric_limits<real_type>::quiet_NaN();
		}
	}


	public: static real_type tier_measure(model_type const& model, uint_type tier_id, performance_measure_category category)
	{
		switch (category)
		{
			case response_time_performance_measure:
				return model.station_response_times()(tier_id);
			default:
				return ::std::numeric_limits<real_type>::quiet_NaN();
		}
	}
};
*/

}} // Namespace dcs::eesim


#endif // DCS_EESIM_APPLICATION_PERFORMANCE_MODEL_TRAITS_HPP
