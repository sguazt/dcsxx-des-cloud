/**
 * \file dcs/perfeval/energy/constant_model.hpp
 *
 * \brief Energy model with constant energy consumption.
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

#ifndef DCS_PERFEVAL_ENERGY_CONSTANT_MODEL_HPP
#define DCS_PERFEVAL_ENERGY_CONSTANT_MODEL_HPP


#include <dcs/macro.hpp>
#include <dcs/perfeval/energy/base_model.hpp>


namespace dcs { namespace perfeval { namespace energy {

/**
 * \brief Energy model with constant energy consumption.
 *
 * \tparam RealT The type for real numbers.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT=double>
class constant_model: public base_model<RealT>
{
	/// Alias for the type of real numbers.
	public: typedef RealT real_type;


	/// A constructor
	public: explicit constant_model(real_type power=real_type/*zero*/())
		: power_(power)
	{
	}


	// Compiler-generated copy ctor and assignement are fine


	private: real_type do_consumed_energy(real_type u) const
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING(u);

		return power_;
	}


	/// The constant energy consumption.
	private: const real_type power_;
};

}}} // Namespace dcs::perfeval::energy


#endif // DCS_PERFEVAL_ENERGY_CONSTANT_MODEL_HPP
