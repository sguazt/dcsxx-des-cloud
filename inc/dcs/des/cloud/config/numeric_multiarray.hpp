/**
 * \file dcs/des/cloud/config/numeric_multiarray.hpp
 *
 * \brief Numeric multi-arrays inside configuration.
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

#ifndef DCS_DES_CLOUD_CONFIG_NUMERIC_MULTIARRAY_HPP
#define DCS_DES_CLOUD_CONFIG_NUMERIC_MULTIARRAY_HPP


#include <algorithm>
#include <cstddef>
#include <dcs/assert.hpp>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <vector>


namespace dcs { namespace des { namespace cloud { namespace config {

namespace detail { namespace /*<unnamed>*/ {

template <typename T, typename ForwardIterT>
void print_container(ForwardIterT first, ForwardIterT last)
{
	::std::cerr << "[";
	::std::copy(first, last, ::std::ostream_iterator<T>(::std::cerr, ","));
	::std::cerr << "]";
}

}} // Namespace detail::<unnamed>


template <typename T>
class numeric_multiarray
{
	public: class iterator;
	public: friend class iterator;
	public: class const_iterator;
	public: friend class const_iterator;

	private: typedef numeric_multiarray<T> self_type;
	public: typedef ::std::size_t size_type;
	public: typedef ::std::ptrdiff_t difference_type;
	public: typedef T value_type;
	public: typedef T* pointer;
	public: typedef T const* const_pointer;
	public: typedef T& reference;
	public: typedef T const& const_reference;
	public: typedef ::std::vector<size_type> index_type;
//	public: typedef T* iterator;
//	public: typedef T const* const_iterator;


	public: numeric_multiarray()
	: dims_(0),
	  data_(0),
	  sz_(0)
	{
	}


	public: template <typename DimsForwardIterT, typename DataForwardIterT, typename ByDimsForwardIterT>
		numeric_multiarray(DimsForwardIterT dim_first, DimsForwardIterT dim_last, DataForwardIterT data_first, DataForwardIterT data_last, ByDimsForwardIterT bydims_first, ByDimsForwardIterT bydims_last)
	: dims_(dim_first, dim_last),
	  data_(0)
	{
		sz_ = ::std::accumulate(dim_first,
								dim_last,
								size_type(1),
								::std::multiplies<size_type>());

		// pre: size == distance of data_last from data_first
		DCS_ASSERT(
			static_cast< ::std::ptrdiff_t >(sz_) == ::std::distance(data_first, data_last),
			throw ::std::invalid_argument("[dcs::des::cloud::config::numeric_multiarray::ctor] Unsufficient number of data.")
		);

		data_ = new value_type[sz_];

		index_type bydims(bydims_first, bydims_last);

		fill_data(data_first, data_last, bydims_first, bydims_last);
	}


	public: numeric_multiarray(numeric_multiarray const& that)
	{
		sz_ = that.size();
		if (sz_ > 0)
		{
			data_ = new value_type[sz_];
			::std::copy(that.data_, that.data_+sz_, data_);
		}
		else
		{
			data_ = 0;
		}

		dims_ = that.dims_;
	}


	public: virtual ~numeric_multiarray()
	{
		if (data_)
		{
			delete[] data_;
		}
	}


	public: numeric_multiarray& operator=(numeric_multiarray const& rhs)
	{
		if (&rhs != this)
		{
			if (data_)
			{
				delete[] data_;
			}
			sz_ = rhs.size();
			if (sz_ > 0)
			{
				data_ = new value_type[sz_];
				::std::copy(rhs.data_, rhs.data_+sz_, data_);
			}
			else
			{
				data_ = 0;
			}

			dims_ = rhs.dims_;
		}
		return *this;
	}


	public: size_type num_dims() const
	{
		return dims_.size();
	}


	public: size_type size(size_type dim) const
	{
		DCS_ASSERT(
			dim < dims_.size(),
			throw ::std::invalid_argument("[dcs::des::cloud::config::numeric_multiarray::size] Dimension out of range.")
		);

		return dims_[dim];
	}


	public: size_type size() const
	{
//		return sizeof(data_)/sizeof(value_type);
		return sz_;
	}


	public: value_type& operator()(index_type const& idx)
	{
		// pre: data != null pointer
		DCS_ASSERT(
			data_,
			throw ::std::runtime_error("[dcs::des::cloud::config::numeric_multiarray::()] Data not defined.")
		);
		// pre: index is valid
		DCS_ASSERT(
			check_index(idx),
			throw ::std::runtime_error("[dcs::des::cloud::config::numeric_multiarray::()] Data not defined.")
		);

		return data_[data_index(idx)];
	}


	public: value_type const& operator()(index_type const& idx) const
	{
		// pre: data != null pointer
		DCS_ASSERT(
			data_,
			throw ::std::runtime_error("[dcs::des::cloud::config::numeric_multiarray::()] Data not defined.")
		);
		// pre: index is valid
		DCS_ASSERT(
			check_index(idx),
			throw ::std::runtime_error("[dcs::des::cloud::config::numeric_multiarray::()] Data not defined.")
		);

		return data_[data_index(idx)];
	}


	public: bool empty() const
	{
		return data_ == 0 || num_dims() == 0;
	}


	private: bool check_index(index_type const& idx) const
	{
		if (idx.size() != dims_.size())
		{
			return false;
		}

		size_type n(idx.size());
		for (size_type i = 0; i < n; ++i)
		{
			if (idx[i] >= dims_[i])
			{
				return false;
			}
		}

		return true;
	}


	private: size_type data_index(index_type const& idx) const
	{
		// Use a row-major like storage layout.

		size_type s(0);
		size_type n(dims_.size());
		for (size_type k = 0; k < n; ++k)
		{
			size_type p(1);

			for (size_type l = k+1; l < n; ++l)
			{
				p *= dims_[l];
			}

			s += p*idx[k];
		}

		return s;
	}


	private: template <typename DataForwardIterT, typename ByDimsForwardIterT>
		void fill_data(DataForwardIterT data_first, DataForwardIterT data_last, ByDimsForwardIterT bydims_first, ByDimsForwardIterT bydims_last)
	{
		index_type bydims(bydims_first, bydims_last);

		index_type idx(dims_.size(), 0);

		while (data_first != data_last)
		{
			size_type i = data_index(idx);

			data_[i] = *data_first;

			increment_index(idx, bydims);

			++data_first;
		}
	}


	void increment_index(index_type& idx, index_type const& bydims, size_type inc = 1) const
	{
		size_type sz(bydims.size());
		size_type carry(inc);
		for (size_type i = 0; i < sz; ++i)
		{
			size_type k(sz - i - 1);
			size_type j(bydims[k]);
			if ((idx[j]+carry) >= dims_[j])
			{
				// Reset counter for this dimension...
				idx[j] = 0;
				// ... And remember the carry for the next digit
				carry = inc;
//				if (k > 0)
//				{
//					idx[bydims[k-1]] += 1;
//				}
			}
			else
			{
				idx[j] += carry;
				break;
			}
		}
	}


	//TODO: check for correctness
	void decrement_index(index_type& idx, index_type const& bydims, size_type dec = 1) const
	{
		size_type sz(bydims.size());
		size_type borrow(dec);
		for (size_type i = 0; i < sz; ++i)
		{
			size_type k(sz - i - 1);
			size_type j(bydims[k]);
			if (idx[j] < borrow)
			{
				// Reset counter for this dimension...
				idx[j] = dims_[j]-1;
				// ... And remember the borrow for the next digit
				borrow = dec;
			}
			else
			{
				idx[j] -= borrow;
				break;
			}
		}
	}


	public: iterator begin()
	{
		size_type nd = num_dims();

		index_type bydims(nd);
		index_type idx(nd, 0);

		for (size_type i = 0; i < nd; ++i)
		{
			bydims[i] = i;
		}

		return iterator(*this, bydims, idx);
	}


	public: iterator end()
	{
		size_type nd = num_dims();

		index_type bydims(nd);
		index_type idx(nd);

		for (size_type i = 0; i < nd; ++i)
		{
			bydims[i] = i;
			idx[i] = dims_[i];
		}

		return iterator(*this, bydims, idx);
	}


	public: iterator begin(index_type const& bydims)
	{
		size_type nd = num_dims();

		index_type idx(nd, 0);

		return iterator(*this, bydims, idx);
	}


	public: iterator end(index_type const& bydims)
	{
		size_type nd = num_dims();

		index_type idx(nd);

		for (size_type i = 0; i < nd; ++i)
		{
			idx[i] = dims_[bydims[i]];
		}

		return iterator(*this, bydims, idx);
	}


	public: const_iterator begin() const
	{
		size_type nd = num_dims();

		index_type bydims(nd);
		index_type idx(nd, 0);

		for (size_type i = 0; i < nd; ++i)
		{
			bydims[i] = i;
		}

		return const_iterator(*this, bydims, idx);
	}


	public: const_iterator end() const
	{
		size_type nd = num_dims();

		index_type bydims(nd);
		index_type idx(nd);

		for (size_type i = 0; i < nd; ++i)
		{
			bydims[i] = i;
			idx[i] = dims_[i];
		}

		return const_iterator(*this, bydims, idx);
	}


	public: const_iterator begin(index_type const& bydims) const
	{
		size_type nd = num_dims();

		index_type idx(nd, 0);

		return const_iterator(*this, bydims, idx);
	}


	public: const_iterator end(index_type const& bydims) const
	{
		size_type nd = num_dims();

		index_type idx(nd);

		for (size_type i = 0; i < nd; ++i)
		{
			idx[i] = dims_[bydims[i]];
		}

		return const_iterator(*this, bydims, idx);
	}


	public: class iterator
	{
		typedef typename self_type::difference_type difference_type;
		typedef typename self_type::value_type value_type;
		typedef typename self_type::reference reference;
		typedef typename self_type::pointer pointer;


		public: iterator(self_type& a, index_type const& bydims, index_type const& idx)
		: a_(a),
		  by_(bydims),
		  idx_(idx),
		  max_idx_(idx.size()),
		  last_idx_(idx.size())
		{
			for (size_type i = 0; i < a_.num_dims(); ++i)
			{
				max_idx_[i] = a_.size(i) > 0 ? (a_.size(i)-1) : 0;
				last_idx_[i] = a_.size(i);
			}
		}


		public: reference operator*() const
		{
			return a_(idx_);
		}


		public: index_type const& index() const
		{
			return idx_;
		}


		public: iterator& operator++()
		{
			if (!equal_indices(idx_, max_idx_))
			{
				a_.increment_index(idx_, by_);
			}
			else
			{
				idx_ = last_idx_;
			}

			return *this;
		}


		public: iterator& operator--()
		{
			if (!equal_indices(idx_, min_idx_))
			{
				a_.decrement_index(idx_, by_);
			}

			return *this;
		}


		//TODO: handle boundary cases (like in op++ and op--)
		public: iterator& operator+=(difference_type n)
		{
			if (n > 0)
			{
				a_.increment_index(idx_, by_, static_cast<size_type>(n));
			}
			else if (n < 0)
			{
				a_.decrement_index(idx_, by_, static_cast<size_type>(-n));
			}

			return *this;
		}


		//TODO: handle boundary cases (like in op++ and op--)
		public: iterator& operator-=(difference_type n)
		{
			if (n > 0)
			{
				a_.decrement_index(idx_, by_, static_cast<size_type>(n));
			}
			else if (n < 0)
			{
				a_.increment_index(idx_, by_, static_cast<size_type>(-n));
			}

			return *this;
		}


		bool operator==(iterator const& it) const
		{
			return equals_indices(idx_, it.idx_);
		}


		bool operator!=(iterator const& it) const
		{
			return !(*this == it);
		}


		bool operator<(iterator const& it) const
		{
			if (idx_.size() < it.idx_.size())
			{
				return true;
			}
			if (idx_.size() > it.idx_.size())
			{
				return false;
			}

			for (size_type i = 0; i < idx_.size(); ++i)
			{
				if (idx_[i] > it.idx_[i])
				{
					return false;
				}
			}

			return true;
		}


		private: bool equal_indices(index_type const& idx1, index_type const& idx2) const
		{
			if (idx1.size() != idx2.size())
			{
				return false;
			}

			for (size_type i = 0; i < idx1.size(); ++i)
			{
				if (idx1[i] != idx2[i])
				{
					return false;
				}
			}

			return true;
		}


		private: self_type& a_;
		private: index_type by_;
		private: index_type idx_;
		private: index_type min_idx_;
		private: index_type max_idx_;
		private: index_type last_idx_;
	}; // iterator


	public: class const_iterator
	{
		typedef typename self_type::difference_type difference_type;
		typedef typename self_type::value_type value_type;
		typedef typename self_type::const_reference reference;
		typedef typename self_type::const_pointer pointer;


		public: const_iterator(self_type const& a, index_type const& bydims, index_type const& idx)
		: a_(a),
		  by_(bydims),
		  idx_(idx),
		  min_idx_(idx.size(), 0),
		  max_idx_(idx.size()),
		  last_idx_(idx.size())
		{
			for (size_type i = 0; i < a_.num_dims(); ++i)
			{
				max_idx_[i] = a_.size(i) > 0 ? (a_.size(i)-1) : 0;
				last_idx_[i] = a_.size(i);
			}
		}


		public: reference operator*() const
		{
			return a_(idx_);
		}


		public: index_type const& index() const
		{
			return idx_;
		}


		public: const_iterator& operator++()
		{
			if (!equal_indices(idx_, max_idx_))
			{
				a_.increment_index(idx_, by_);
			}
			else
			{
				idx_ = last_idx_;
			}

			return *this;
		}


		public: const_iterator& operator--()
		{
			if (!equal_indices(idx_, min_idx_))
			{
				a_.decrement_index(idx_, by_);
			}

			return *this;
		}


		//TODO: handle boundary cases (like in op++ and op--)
		public: const_iterator& operator+=(difference_type n)
		{
			if (n > 0)
			{
				a_.increment_index(idx_, by_, static_cast<size_type>(n));
			}
			else if (n < 0)
			{
				a_.decrement_index(idx_, by_, static_cast<size_type>(-n));
			}

			return *this;
		}


		//TODO: handle boundary cases (like in op++ and op--)
		public: const_iterator& operator-=(difference_type n)
		{
			if (n > 0)
			{
				a_.decrement_index(idx_, by_, static_cast<size_type>(n));
			}
			else if (n < 0)
			{
				a_.increment_index(idx_, by_, static_cast<size_type>(-n));
			}

			return *this;
		}


		bool operator==(const_iterator const& it) const
		{
			return equal_indices(idx_, it.idx_);
		}


		bool operator!=(const_iterator const& it) const
		{
			return !(*this == it);
		}


		bool operator<(const_iterator const& it) const
		{
			if (idx_.size() < it.idx_.size())
			{
				return true;
			}
			if (idx_.size() > it.idx_.size())
			{
				return false;
			}

			for (size_type i = 0; i < idx_.size(); ++i)
			{
				if (idx_[i] > it.idx_[i])
				{
					return false;
				}
			}

			return true;
		}


		private: bool equal_indices(index_type const& idx1, index_type const& idx2) const
		{
			if (idx1.size() != idx2.size())
			{
				return false;
			}

			for (size_type i = 0; i < idx1.size(); ++i)
			{
				if (idx1[i] != idx2[i])
				{
					return false;
				}
			}

			return true;
		}


		private: self_type const& a_;
		private: index_type by_;
		private: index_type idx_;
		private: index_type min_idx_;
		private: index_type max_idx_;
		private: index_type last_idx_;
	}; // const_iterator


	private: index_type dims_;
	private: value_type* data_;
	private: size_type sz_;
};


namespace detail { namespace /*<unnamed>*/ {

template <typename CharT, typename CharTraitsT, typename ValueT>
void print_array_r(::std::basic_ostream<CharT,CharTraitsT>& os, numeric_multiarray<ValueT> const& a, typename numeric_multiarray<ValueT>::index_type& idx, ::std::size_t dim)
{
	if (dim == (a.num_dims()-1))
	{
		for (::std::size_t i = 0; i < a.size(dim); ++i)
		{
			if (i > 0)
			{
				os << " ";
			}

			idx[dim] = i;

			os << a(idx);
		}
	}
	else
	{
		for (::std::size_t i = 0; i < a.size(dim); ++i)
		{
			if (i > 0)
			{
				os << "; ";
			}

			os << "[";

			idx[dim] = i;

			print_array_r(os, a, idx, dim+1);

			os << "]";
		}
	}

	idx[dim] = 0;
}

template <typename CharT, typename CharTraitsT, typename ValueT>
void print_array(::std::basic_ostream<CharT,CharTraitsT>& os, numeric_multiarray<ValueT> const& a)
{
	typename numeric_multiarray<ValueT>::index_type idx(a.num_dims(), 0);

	if (a.size() > 0)
	{
		print_array_r(os, a, idx, 0);
	}
}

}} // Namespace detail::<unnamed>

template <typename CharT, typename CharTraitsT, typename ValueT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, numeric_multiarray<ValueT> const& a)
{
	os << "[";

	detail::print_array(os, a);

	os << "]";

	return os;
}

}}}} // Namespace dcs::des::cloud::config


#endif // DCS_DES_CLOUD_CONFIG_NUMERIC_MULTIARRAY_HPP
