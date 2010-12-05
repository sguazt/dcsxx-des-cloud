#ifndef DCS_EESIM_CONFIG_NUMERIC_MATRIX_HPP
#define DCS_EESIM_CONFIG_NUMERIC_MATRIX_HPP


#include <algorithm>
#include <cstddef>
#include <dcs/assert.hpp>
#include <iostream>
#include <iterator>
#include <stdexcept>


namespace dcs { namespace eesim { namespace config {

template <typename T>
class numeric_matrix
{
	public: typedef ::std::size_t size_type;
	public: typedef T value_type;


	public: numeric_matrix()
	: nr_(0),
	  nc_(0),
	  data_(0)
	{
	}


	public: template <typename ForwardIterT>
		numeric_matrix(size_type r, size_type c, ForwardIterT data_first, ForwardIterT data_last, bool byrow)
		: nr_(r),
		  nc_(c),
		  data_(0)
		{
			// pre: #rows > 0
			DCS_ASSERT(
				nr_ > 0,
				throw ::std::invalid_argument("[dcs::eesim::config::numeric_matrix::ctor] Invalid number of rows.")
			);
			// pre: #columns > 0
			DCS_ASSERT(
				nc_ > 0,
				throw ::std::invalid_argument("[dcs::eesim::config::numeric_matrix::ctor] Invalid number of columns.")
			);
			// pre: size == distance of data_last from data_first
			DCS_ASSERT(
				static_cast< ::std::ptrdiff_t >(nr_*nc_) == ::std::distance(data_first, data_last),
				throw ::std::invalid_argument("[dcs::eesim::config::numeric_matrix::ctor] Unsufficient number of data.")
			);

			size_type sz = nr_*nc_;
			//::std::ptrdiff_t datasz = ::std::distance(data_first, data_last);

			data_ = new value_type[sz];
			if (!data_)
			{
				throw ::std::runtime_error("[dcs::eesim::config::numeric_matrix::ctor] Insufficient space for storing the matrix.");
			}

			if (byrow)
			{
				for (size_type r = 0; r < nr_; ++r)
				{
					for (size_type c = 0; c < nc_; ++c)
					{
						if (data_first != data_last)
						{
							data_[index(r,c)] = *data_first;
							++data_first;
						}
						else
						{
							data_[index(r,c)] = value_type/*zero*/();
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
							data_[index(r,c)] = *data_first;
							++data_first;
						}
						else
						{
							data_[index(r,c)] = value_type/*zero*/();
						}
					}
				}
			}
		}

	public: numeric_matrix(numeric_matrix const& that)
	{
		nr_ = that.nr_;
		nc_ = that.nc_;
		size_type sz = nr_*nc_;
		data_ = new value_type[sz];
		if (!data_)
		{
			throw ::std::runtime_error("[dcs::eesim::config::numeric_matrix::copy_ctor] Insufficient space for storing the matrix.");
		}
		::std::copy(that.data_, that.data_+sz, data_);
	}


	public: virtual ~numeric_matrix()
	{
		if (data_)
		{
			delete[] data_;
		}
	}


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
			if (!data_)
			{
				throw ::std::runtime_error("[dcs::eesim::config::numeric_matrix::=] Insufficient space for storing the matrix.");
			}
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
			throw ::std::runtime_error("[dcs::des::eesim::config::numeric_matrix::()] Data not defined.")
		);
		// pre: r < #rows
		DCS_ASSERT(
			r < nr_,
			throw ::std::invalid_argument("[dcs::des::eesim::config::numeric_matrix::()] Row out of range.")
		);
		// pre: c < #columns
		DCS_ASSERT(
			c < nc_,
			throw ::std::invalid_argument("[dcs::des::eesim::config::numeric_matrix::()] Columns out of range.")
		);

		return data_[index(r,c)];
	}


	public: value_type const& operator()(size_type r, size_type c) const
	{
		// pre: data != null pointer
		DCS_ASSERT(
			data_,
			throw ::std::runtime_error("[dcs::des::eesim::config::numeric_matrix::()] Data not defined.")
		);
		// pre: r < #rows
		DCS_ASSERT(
			r < nr_,
			throw ::std::invalid_argument("[dcs::des::eesim::config::numeric_matrix::()] Row out of range.")
		);
		// pre: c < #columns
		DCS_ASSERT(
			c < nc_,
			throw ::std::invalid_argument("[dcs::des::eesim::config::numeric_matrix::()] Columns out of range.")
		);

		return data_[index(r,c)];
	}


	public: bool empty() const
	{
		return nr_ == 0 || nc_ == 0;
	}


	private: size_type index(size_type r, size_type c) const
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

}}} // Namespace dcs::eesim::config


#endif // DCS_EESIM_CONFIG_NUMERIC_MATRIX_HPP
