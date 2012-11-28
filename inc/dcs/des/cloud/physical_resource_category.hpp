/**
 * \file dcs/des/cloud/physical_resource_category.hpp
 *
 * \brief Categories for physical resources.
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

#ifndef DCS_DES_CLOUD_PHYSICAL_RESOURCE_CATEGORY_HPP
#define DCS_DES_CLOUD_PHYSICAL_RESOURCE_CATEGORY_HPP


namespace dcs { namespace des { namespace cloud {

enum physical_resource_category
{
	cpu_resource_category,
	memory_resource_category,
	storage_resource_category
	//network_resource_category
	//network_up_resource_category,
	//network_down_resource_category,
};

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_PHYSICAL_RESOURCE_CATEGORY_HPP
