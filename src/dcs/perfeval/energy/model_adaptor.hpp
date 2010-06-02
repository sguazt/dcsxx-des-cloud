/**
 * \file dcs/perfeval/energy/model_adaptor.hpp
 *
 * \brief Adaptor for energy model classes.
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
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_PERFEVAL_ENERGY_MODEL_ADAPTOR_HPP
#define DCS_PERFEVAL_ENERGY_MODEL_ADAPTOR_HPP


#include <dcs/perfeval/energy/base_model.hpp>
#include <dcs/type_traits/add_const.hpp>
#include <dcs/type_traits/add_reference.hpp>


namespace dcs { namespace perfeval { namespace energy {

template <
	typename EnergyModelT,
	typename EnergyModelTraitsT=typename ::dcs::type_traits::remove_reference<EnergyModelT>::type
>
class model_adaptor: public base_model<typename EnergyModelTraitsT::real_type>
{
	public: typedef typename EnergyModelTraitsT::real_type real_type;
	private: typedef EnergyModelT model_type;
	public: typedef typename ::dcs::type_traits::add_reference<EnergyModelT>::type model_reference;
	public: typedef typename ::dcs::type_traits::add_reference<typename ::dcs::type_traits::add_const<EnergyModelT>::type>::type model_const_reference;


	public: model_adaptor(model_const_reference model)
		: model_(model)
	{
		// empty
	}


	public: real_type consumed_energy(real_type u) const
	{
		return model_.consumed_energy(u);
	}


	private: model_type model_;
};

}}} // Namespace dcs::perfeval::energy


#endif // DCS_PERFEVAL_ENERGY_MODEL_ADAPTOR_HPP
