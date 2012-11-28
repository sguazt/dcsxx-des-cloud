/**
 * \file dcs/des/cloud/config/numeric_matrix.hpp
 *
 * \brief Numeric matrices inside configuration.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright (C) 2009-2012  Marco Guazzone (marco.guazzone@gmail.com)
 *                          [Distributed Computing System (DCS) Group,
 *                           Computer Science Institute,
 *                           Department of Science and Technological Innovation,
 *                           University of Piemonte Orientale,
 *                           Alessandria (Italy)]
 *
 * This file is part of dcsxx-des-cloud.
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
 */

#ifndef DCS_DES_CLOUD_CONFIG_NUMERIC_MATRIX_HPP
#define DCS_DES_CLOUD_CONFIG_NUMERIC_MATRIX_HPP


#include <algorithm>
#include <cstddef>
#include <dcs/assert.hpp>
#include <iosfwd>
#include <iterator>
#include <stdexcept>


namespace dcs { namespace des { namespace cloud { namespace config {

template <typename T>
class numeric_matrix
{
	public: typedef ::std::size_t size_type;
	public: typedef T value_type;


	/// A constructor
	public: numeric_matrix()
	: nr_(0),
	  nc_(0),
	  data_(0)
	{
	}


	/// A constructor
	public: template <typename ForwardIterT>
		numeric_matrix(size_type r, size_type c, ForwardIterT data_first, ForwardIterT data_last, bool byrow)
		: nr_(r),
		  nc_(c),
		  data_(0)
	{
		// pre: #rows > 0
		DCS_ASSERT(
			nr_ > 0,
			throw ::std::invalid_argument("[dcs::des::cloud::config::numeric_matrix::ctor] Invalid number of rows.")
		);
		// pre: #columns > 0
		DCS_ASSERT(
			nc_ > 0,
			throw ::std::invalid_argument("[dcs::des::cloud::config::numeric_matrix::ctor] Invalid number of columns.")
		);
		// pre: size == distance of data_last from data_first
		DCS_ASSERT(
			static_cast< ::std::ptrdiff_t >(nr_*nc_) == ::std::distance(data_first, data_last),
			throw ::std::invalid_argument("[dcs::des::cloud::config::numeric_matrix::ctor] Unsufficient number of data.")
		);

		size_type sz = nr_*nc_;
		//::std::ptrdiff_t datasz = ::std::distance(data_first, data_last);

		data_ = new value_type[sz];

		if (byrow)
		{
			for (size_type r = 0; r < nr_; ++r)
			{
				for (size_type c = 0; c < nc_; ++c)
				{
					if (data_first != data_last)
					{
						data_[make_index(r,c)] = *data_first;
						++data_first;
					}
					else
					{
						data_[make_index(r,c)] = value_type/*zero*/();
					}
				}
			}
		}
		else
		{
			for (size_type c = 0; c < nc_; ++c)
			{
				for (size_type r = 0; r < nr_; ++r)
				{
					if (data_first != data_last)
					{
						data_[make_index(r,c)] = *data_first;
						++data_first;
					}
					else
					{
						data_[make_index(r,c)] = value_type/*zero*/();
					}
				}
			}
		}
	}


	/// A constructor
	public: numeric_matrix(size_type r, size_type c, value_type v)
		: nr_(r),
		  nc_(c),
		  data_(0)
	{
		// pre: #rows > 0
		DCS_ASSERT(
			nr_ > 0,
			throw ::std::invalid_argument("[dcs::des::cloud::config::numeric_matrix::ctor] Invalid number of rows.")
		);
		// pre: #columns > 0
		DCS_ASSERT(
			nc_ > 0,
			throw ::std::invalid_argument("[dcs::des::cloud::config::numeric_matrix::ctor] Invalid number of columns.")
		);

		size_type sz = nr_*nc_;

		data_ = new value_type[sz];

		::std::fill(data_, data_+sz, v);
	}


	/// Copy constructor.
	public: numeric_matrix(numeric_matrix const& that)
	{
		nr_ = that.nr_;
		nc_ = that.nc_;
		size_type sz = nr_*nc_;
		data_ = new value_type[sz];
		::std::copy(that.data_, that.data_+sz, data_);
	}


	/// The destructor.
	public: virtual ~numeric_matrix()
	{
		if (data_)
		{
			delete[] data_;
		}
	}


	/// The copy assignement.
	public: numeric_matrix& operator=(numeric_matrix const& rhs)
	{
		if (&rhs != this)
		{
			nr_ = rhs.nr_;
			nc_ = rhs.nc_;
			if (data_)
			{
				delete[] data_;
			}
			size_type sz = nr_*nc_;
			data_ = new value_type[sz];
			::std::copy(rhs.data_, rhs.data_+sz, data_);
		}
		return *this;
	}


	public: size_type num_rows() const
	{
		return nr_;
	}


	public: size_type num_columns() const
	{
		return nc_;
	}


	public: value_type& operator()(size_type r, size_type c)
	{
		// pre: data != null pointer
		DCS_ASSERT(
			data_,
			throw ::std::runtime_error("[dcs::des::cloud::config::numeric_matrix::()] Data not defined.")
		);
		// pre: r < #rows
		DCS_ASSERT(
			r < nr_,
			throw ::std::invalid_argument("[dcs::des::cloud::config::numeric_matrix::()] Row out of range.")
		);
		// pre: c < #columns
		DCS_ASSERT(
			c < nc_,
			throw ::std::invalid_argument("[dcs::des::cloud::config::numeric_matrix::()] Columns out of range.")
		);

		return data_[make_index(r,c)];
	}


	public: value_type const& operator()(size_type r, size_type c) const
	{
		// pre: data != null pointer
		DCS_ASSERT(
			data_,
			throw ::std::runtime_error("[dcs::des::cloud::config::numeric_matrix::()] Data not defined.")
		);
		// pre: r < #rows
		DCS_ASSERT(
			r < nr_,
			throw ::std::invalid_argument("[dcs::des::cloud::config::numeric_matrix::()] Row out of range.")
		);
		// pre: c < #columns
		DCS_ASSERT(
			c < nc_,
			throw ::std::invalid_argument("[dcs::des::cloud::config::numeric_matrix::()] Columns out of range.")
		);

		return data_[make_index(r,c)];
	}


	public: bool empty() const
	{
		return nr_ == 0 || nc_ == 0;
	}


	private: size_type make_index(size_type r, size_type c) const
	{
		// Use the row-major order
		return r*nc_+c;
	}


	private: size_type nr_;
	private: size_type nc_;
	private: value_type* data_;
};


template <typename CharT, typename CharTraitsT, typename ValueT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, numeric_matrix<ValueT> const& m)
{
	os << "[";

	::std::size_t nr = m.num_rows();
	::std::size_t nc = m.num_columns();
	for (::std::size_t r = 0; r < nr; ++r)
	{
		if (r != 0)
		{
			os << ";";
		}
		for (::std::size_t c = 0; c < nc; ++c)
		{
//			if (c != 0)
//			{
				os << " ";
//			}
			os << m(r,c);
		}
	}

	os << "]";

	return os;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_NUMERIC_MATRIX_HPP
