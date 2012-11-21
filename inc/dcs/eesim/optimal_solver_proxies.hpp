/**
 * \file dcs/eesim/optimal_solver_proxies.hpp
 *
 * \brief Categories for optimal mathematical solver proxies.
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

#ifndef DCS_EESIM_OPTIMAL_SOLVER_PROXIES_HPP
#define DCS_EESIM_OPTIMAL_SOLVER_PROXIES_HPP


namespace dcs { namespace eesim {

enum optimal_solver_proxies
{
	//kestrel_optimal_solver_proxy,
	neos_optimal_solver_proxy,
	none_optimal_solver_proxy
};

}} // Namespace dcs::eesim


#endif // DCS_EESIM_OPTIMAL_SOLVER_PROXIES_HPP
