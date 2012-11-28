#ifndef DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_ALGEBRAIC_TYPE_HPP
#define DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_ALGEBRAIC_TYPE_HPP


#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cstddef>
#include <dcs/des/cloud/config/numeric_matrix.hpp>
//#include <dcs/des/cloud/config/numeric_multiarray.hpp>
#include <iterator>
#include <vector>


namespace dcs { namespace des { namespace cloud { namespace config {

template <typename T>
::boost::numeric::ublas::matrix<T> make_ublas_matrix(numeric_matrix<T> const& m)
{
	typedef typename numeric_matrix<T>::size_type size_type;

	size_type nr = m.num_rows();
	size_type nc = m.num_columns();

	::boost::numeric::ublas::matrix<T> res(nr, nc);

	for (size_type r = 0; r < nr; ++r)
	{
		for (size_type c = 0; c < nc; ++c)
		{
			res(r,c) = m(r,c);
		}
	}

	return res;
}


template <typename T>
::boost::numeric::ublas::vector<T> make_ublas_vector(::std::vector<T> const& v)
{
	typedef typename ::std::vector<T>::size_type size_type;

	size_type n = v.size();

	::boost::numeric::ublas::vector<T> res(n);

	for (size_type i = 0; i < n; ++i)
	{
		res(i) = v[i];
	}

	return res;
}


/*TODO: test-me
template <typename V>
::boost::numeric::ublas::vector<typename V::value_type> make_ublas_vector(V const& v)
{
	typedef typename V::size_type size_type;
	typedef typename V::value_type value_type;

	size_type n = v.size();

	::boost::numeric::ublas::vector<value_type> res(n);

	for (size_type i = 0; i < n; ++i)
	{
		res(i) = v[i];
	}

	return res;
}
*/


template <typename ForwardIterT>
::boost::numeric::ublas::vector<
	typename ::std::iterator_traits<ForwardIterT>::value_type
> make_ublas_vector(ForwardIterT first, ForwardIterT last)
{
	typedef typename ::std::iterator_traits<ForwardIterT>::value_type value_type;

	::boost::numeric::ublas::vector<value_type> res;

	for (::std::size_t i = 0; first != last; ++i)
	{
		res.resize(res.size()+1);
		res(i) = *first;
		++first;
	}

	return res;
}

}}}} // Namespace dcs::des::cloud::config

#endif // DCS_DES_CLOUD_CONFIG_OPERATION_MAKE_ALGEBRAIC_TYPE_HPP
