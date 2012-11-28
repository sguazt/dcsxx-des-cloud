/**
 * \file dcs/des/cloud/power_status.hpp
 *
 * \brief Power statuses.
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

#ifndef DCS_DES_CLOUD_POWER_STATUS_HPP
#define DCS_DES_CLOUD_POWER_STATUS_HPP


#include <iostream>


namespace dcs { namespace des { namespace cloud {

enum power_status
{
	powered_on_power_status,
	powered_off_power_status,
	suspended_power_status
};


template <
	typename CharT,
	typename CharTraitsT
>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, power_status status)
{
	switch (status)
	{
		case powered_on_power_status:
			os << "powered on";
			break;
		case powered_off_power_status:
			os << "powered off";
			break;
		case suspended_power_status:
			os << "suspended";
			break;
		default:
			os << "unknown";
	}

	return os;
}

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_POWER_STATUS_HPP
