/**
 * \file dcs/des/cloud/identifier_generator.hpp
 *
 * \brief Entity identifiers generator.
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

#ifndef DCS_DES_CLOUD_IDENTIFIER_GENERATOR_HPP
#define DCS_DES_CLOUD_IDENTIFIER_GENERATOR_HPP


namespace dcs { namespace des { namespace cloud {

template <typename T>
class identifier_generator
{
	public: typedef T value_type;


	public: explicit identifier_generator(value_type x0 = value_type/*zero*/())
	: x_(x0)
	{
	}


	public: value_type operator()()
	{
		return x_++;
	}


	public: void reset(value_type x0 = value_type/*zero*/())
	{
		x_ = x0;
	}


	private: value_type x_;
};

}}} // Namespace dcs::des::cloud


#endif // DCS_DES_CLOUD_IDENTIFIER_GENERATOR_HPP
