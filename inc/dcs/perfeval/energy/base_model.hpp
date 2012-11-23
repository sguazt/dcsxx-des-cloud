/*
 * \file dcs/perfeval/energy/base_model.hpp
 *
 * \brief Base class for energy models implementing the EnergyModel concept.
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

#ifndef DCS_PERFEVAL_ENERGY_BASE_MODEL_HPP
#define DCS_PERFEVAL_ENERGY_BASE_MODEL_HPP


namespace dcs { namespace perfeval { namespace energy {

/**
 * \brief Base class for energy models implementing the EnergyModel concept.
 *
 * \tparam RealT The type for real numbers.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT>
class base_model
{
	//[TODO]
	//DCS_CONCEPT_ASSERT((EnergyModel<base_model<RealT>))
	//[/TODO]

	/// Alias for the type of real numbers.
	public: typedef RealT real_type;

	/// The virtual destructor to make inherited classes destructible.
	public: virtual ~base_model() { }

	/**
	 * \brief Compute the energy consumed for the given system utilization.
	 * \param u The system utilization.
	 * \return The energy consumed for the given system utilization.
	 */
//	public: virtual real_type consumed_energy(real_type u) const = 0;
	public: real_type consumed_energy(real_type u) const
	{
		return do_consumed_energy(u);
	}

	private: virtual real_type do_consumed_energy(real_type u) const = 0;
};

}}} // Namespace dcs::perfeval::energy


#endif // DCS_PERFEVAL_ENERGY_BASE_MODEL_HPP
