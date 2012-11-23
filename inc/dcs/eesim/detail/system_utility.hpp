/**
 * \file dcs/eesim/detail/system_utility.hpp
 *
 * \brief Utilities to access to system-level functionalities.
 *
 * Copyright (C) 2009-2012  Distributed Computing System (DCS) Group, Computer
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

#ifndef DCS_EESIM_DETAIL_SYSTEM_UTILITY_HPP
#define DCS_EESIM_DETAIL_SYSTEM_UTILITY_HPP


#include <cstdlib>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>


namespace dcs { namespace eesim { namespace detail {

::std::string make_tmp_file(::std::string name)
{
    name += "XXXXXX";
    ::std::vector<char> tmpl(name.begin(), name.end());
    tmpl.push_back('\0');

    int fd = ::mkstemp(&tmpl[0]);
    if (fd != -1)
    {
        name.assign(tmpl.begin(), tmpl.end()-1);
        ::close(fd);
    }

    return name;
}


inline
::std::string make_tmp_file()
{
	return make_tmp_file(::std::string("tmpXXXXXX"));
}


::std::string make_tmp_file(::std::string name, ::std::ofstream& of)
{
    name += "XXXXXX";
    ::std::vector<char> tmpl(name.begin(), name.end());
    tmpl.push_back('\0');

    int fd = ::mkstemp(&tmpl[0]);
    if (fd != -1)
    {
        name.assign(tmpl.begin(), tmpl.end()-1);
        of.open(name.c_str(), ::std::ios_base::trunc | ::std::ios_base::out);
        ::close(fd);
    }

    return name;
}


inline
::std::string make_tmp_file(::std::string path, ::std::string name, ::std::ofstream& of)
{
    path += "/" + name;

    return make_tmp_file(path, of);
}

}}} // Namespace dcs::eesim::detail


#endif // DCS_EESIM_DETAIL_SYSTEM_UTILITY_HPP
