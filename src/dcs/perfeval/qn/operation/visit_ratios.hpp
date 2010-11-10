/**
 * \file dcs/perfeval/qn/operation/visit_ratios.hpp
 *
 * \brief Compute the visit ratios of a Queueing Network..
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
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */

#ifndef DCS_PERFEVAL_QN_OPERATION_VISIT_RATIOS_HPP
#define DCS_PERFEVAL_QN_OPERATION_VISIT_RATIOS_HPP


#include <dcs/assert.hpp>
//#include <dcs/math/la/container/identity_matrix.hpp>
//#include <dcs/math/la/operation/lu.hpp>
//#include <dcs/math/la/operation/matrix_basic_operations.hpp>
//#include <dcs/math/la/operation/num_rows.hpp>
//#include <dcs/math/la/operation/subrange.hpp>
//#include <dcs/math/la/operation/sum.hpp>
//#include <dcs/math/la/operation/vector_basic_operations.hpp>
//#include <dcs/math/la/traits/matrix.hpp>
//#include <dcs/math/la/traits/promote.hpp>
//#include <dcs/math/la/traits/vector.hpp>
#include <boost/numeric/ublas/detail/temporary.hpp>
#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublasx/operation/lu.hpp>
#include <boost/numeric/ublasx/operation/num_rows.hpp>
#include <boost/numeric/ublasx/operation/lsq.hpp>
#include <boost/numeric/ublasx/operation/lu.hpp>
#include <boost/numeric/ublasx/operation/sum.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/numeric/ublas/traits.hpp>
#include <boost/multi_array.hpp>
#include <stdexcept>


namespace dcs { namespace perfeval { namespace qn {

/**
 * \brief Compute the single-class visit ratios for an open queueing network.
 *
 * Visit ratios are computed according to the <em>Visit Ratios Equatios</em>
 * [1,2] for which:
 * \f{align}{
 *   V_0 &= 1 \\
 *   V_j &= p_{0j}+\sum_{i=1}^K V_i p_{ij}, \quad j=1,\ldots,K
 * \f}
 * where:
 * - \f$p_{ij}\f$ is the <em>routing probability</em>, that is the fraction of
 *   jobs proceeding next to station \f$j\f$ on completing a service request at
 *   station \f$j\f$;
 * - \f$p_{0j}\lambda_j\f$ is the input routing probability, corresponding to
 *   the arrival rate at station \f$j\f$, that is to the fraction of arrivals to
 *   the system which proceed first to station \f$j\f$.
 * .
 *
 * References:
 * - [1] P. Denning et al.
 *       "The Operational Analysis of Queueing Network Models".
 *       Computing Surveys 10(3):225-261, 1978
 * - [2] G. Bolch et al.
 *       "Queueing Networks and Markov Chains".
 *       John Wiley & Sons, Inc. 1998.
 * .
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename MatrixExprT, typename VectorExprT>
typename ::boost::numeric::ublas::vector_temporary_traits<VectorExprT>::type visit_ratios(::boost::numeric::ublas::matrix_expression<MatrixExprT> const& P,
																					  ::boost::numeric::ublas::vector_expression<VectorExprT> const& lambda)
{
	typedef typename ::boost::numeric::ublas::vector_temporary_traits<VectorExprT>::type vector_type;
	typedef typename ::boost::numeric::ublas::matrix_temporary_traits<MatrixExprT>::type matrix_type;
	typedef typename ::boost::numeric::ublas::promote_traits<
					typename ::boost::numeric::ublas::matrix_traits<MatrixExprT>::size_type,
					typename ::boost::numeric::ublas::vector_traits<VectorExprT>::size_type
				>::promote_type size_type;

	typedef typename ::boost::numeric::ublas::promote_traits<
					typename ::boost::numeric::ublas::matrix_traits<MatrixExprT>::value_type,
					typename ::boost::numeric::ublas::vector_traits<VectorExprT>::value_type
				>::promote_type value_type;

	size_type n = ::boost::numeric::ublas::num_rows(P);

	// precondition: n == num_columns(P)
	DCS_ASSERT(
		n == ::boost::numeric::ublasx::num_columns(P),
		throw ::std::invalid_argument("[dcs::perfeval::queue::qnet::visit_ratios] Probability transitions matrix must be a square matrix.")
	);

	// precondition: n == size(lambda)
	DCS_ASSERT(
		n == ::boost::numeric::ublasx::size(lambda),
		throw ::std::invalid_argument("[dcs::perfeval::queue::qnet::visit_ratios] Arrival rates vector and probability transitions matrix are of non-conformant size.")
	);

	// precondition: all(lambda > 0)
	DCS_ASSERT(
		::boost::numeric::ublasx::all(
			lambda,
			::std::bind2nd(::std::greater_equal<value_type>(), value_type(0))
		),
		throw ::std::invalid_argument("[dcs::perfeval::queue::qnet::visit_ratios] Arrival rates must be non-negative values.")
	);

	// precondition: all(P >= 0) && all(sum_rows(P) <= 1+1.0e-5)
	DCS_ASSERT(
		::boost::numeric::ublasx::all(
			P,
			::std::bind2nd(::std::greater_equal<value_type>(), value_type(0))
		)
		&&
		::boost::numeric::ublasx::all(
			::boost::numeric::ublasx::sum_rows(P),
			::std::bind2nd(::std::less_equal<value_type>(), value_type(1+1.0e-5))
		),
		throw ::std::invalid_argument("[dcs::perfeval::queue::qnet::visit_ratios] Probability transitions matrix is not a stochastic matrix for closed networks.")
	);

	matrix_type A(n, n, 0);
	vector_type b(n, 0);

	// A = [ I_n - P ]
	A = ::boost::numeric::ublas::identity_matrix<value_type>(n) - P;

	// b = lambda / \sum_i lambda_i
	b = lambda / ::boost::numeric::ublasx::sum(lambda);

	vector_type v(n);

	// Find V = A/b = (A'\b')'
	A = ::boost::numeric::ublas::trans(A);
	//b = ::boost::numeric::ublas::trans(b);
	size_type singular = ::boost::numeric::ublasx::lu_solve_inplace(A, b);
	//if (!singular)
	//{
	//	v = ::boost::numeric::ublas::trans(b);
	//}
	//else
	//{
	//	v = vector_type();
	//}
	if (singular)
	{
		b = vector_type();
	}

	//return v;
	return b;
}


/**
 * \brief Compute the single-class visit ratios for a closed queueing network.
 *
 * Visit ratios are computed according to the <em>Visit Ratios Equatios</em>
 * [1] for which:
 * \f{align}{
 *   V_j &= \sum_{i=1}^K V_i p_{ij}, \quad j=1,\ldots,K
 * \f}
 * where:
 * - \f$p_{ij}\f$ is the <em>routing probability</em>, that is the fraction of
 *   jobs proceeding next to station \f$j\f$ on completing a service request at
 *   station \f$j\f$.
 * .
 * Since there are only \f$(K-1)\f$ independent equations for the visit ratios
 * in closed networks, the \f$V_j\f$ can only be determined up to a
 * multiplicative constant.
 * Usually one assume that \f$V_1=1\f$, although other possibilities are used as
 * well.
 * In this case, the solution is found in the least-square sense.
 *
 * References:
 * - [1] G. Bolch et al.
 *       "Queueing Networks and Markov Chains".
 *       John Wiley & Sons, Inc. 1998.
 * .
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
template <typename VectorExprT, typename MatrixExprT>
typename ::boost::numeric::ublas::vector_temporary_traits<VectorExprT>::type visit_ratios(::boost::numeric::ublas::matrix_expression<MatrixExprT> const& P)
{
	typedef typename ::boost::numeric::ublas::vector_temporary_traits<VectorExprT>::type vector_type;
	typedef typename ::boost::numeric::ublas::matrix_temporary_traits<MatrixExprT>::type matrix_type;
	typedef typename ::boost::numeric::ublas::promote_traits<
					typename ::boost::numeric::ublas::matrix_traits<MatrixExprT>::size_type,
					typename ::boost::numeric::ublas::vector_traits<VectorExprT>::size_type
				>::promote_type size_type;
	typedef typename ::boost::numeric::ublas::promote_traits<
					typename ::boost::numeric::ublas::matrix_traits<MatrixExprT>::value_type,
					typename ::boost::numeric::ublas::vector_traits<VectorExprT>::value_type
				>::promote_type value_type;

	size_type n = ::boost::numeric::ublasx::num_rows(P);

	DCS_ASSERT(
		n == ::boost::numeric::ublasx::num_columns(P),
		throw ::std::invalid_argument("[dcs::perfeval::queue::qnet::visit_ratios] Probability transitions matrix must be a square matrix.")
	);

	DCS_ASSERT(
		::boost::numeric::ublasx::all(
			P,
			::std::bind2nd(::std::greater_equal<value_type>(), value_type(0))
		)
		&&
		::boost::numeric::ublasx::all(
			::boost::numeric::ublasx::sum_rows(P),
			::std::bind2nd(::std::less_equal<value_type>(), value_type(1+1e-5))
		),
		throw ::std::invalid_argument("[dcs::perfeval::queue::qnet::visit_ratios] Probability transitions matrix is not a stochastic matrix for closed networks.")
	);

	matrix_type A(n, n+1, 0);
	vector_type b(n+1, 0);

	// A = [ P - I_n; 0 ... 0 1 ] 
	::boost::numeric::ublas::subrange(A, 0, n, 0, n) = P - ::boost::numeric::ublas::identity_matrix<value_type>(n);
	A(0, n) = value_type(1);

	// b = [ 0 ... 0 1 ]
	b(n) = value_type(1);

	vector_type v(n);

	// Find v = A/b = (A'\b')'
	A = ::boost::numeric::ublas::trans(A);
	//b = ::boost::numeric::ublas::trans(b);
	v = ::boost::numeric::ublasx::llsq(A, b);
	//v = ::boost::numeric::ublas::trans(v);

	return v;
}


/**
 * \brief Compute the multi-class visit ratios for an open queueing network.
 *
 * Visit ratios are computed according to the <em>Visit Ratios Equatios</em>
 * [1,2] for which:
 * \f{align}{
 *   V_0 &= 1 \\
 *   V_j &= p_{0j}+\sum_{i=1}^K V_i p_{ij}, \quad j=1,\ldots,K
 * \f}
 * where:
 * - \f$p_{ij}\f$ is the <em>routing probability</em>, that is the fraction of
 *   jobs proceeding next to station \f$j\f$ on completing a service request at
 *   station \f$j\f$;
 * - \f$p_{0j}\lambda_j\f$ is the input routing probability, corresponding to
 *   the arrival rate at station \f$j\f$, that is to the fraction of arrivals to
 *   the system which proceed first to station \f$j\f$.
 * .
 *
 * References:
 * - [1] P. Denning et al.
 *       "The Operational Analysis of Queueing Network Models".
 *       Computing Surveys 10(3):225-261, 1978
 * - [2] G. Bolch et al.
 *       "Queueing Networks and Markov Chains".
 *       John Wiley & Sons, Inc. 1998.
 * .
 *
 * \author Marco Guazzone, &lt;marco.guazzone@mfn.unipmn.it&gt;
 */
/*TODO: here below is the pseudo code. The main idea is to start from the multidimensional array P of size nn x nc x nn x nc where nn is the number of stations and nc is the number of class and P(i,j,k,h) is the probability the a j-class customer at station j go to station k and change its class to h.
 *
template <typename ValueT, typename MatrixExprT>
typename ::boost::numeric::ublas::matrix_temporary_traits<MatrixExprT>::type visit_ratios(::boost::multi_array<ValueT, 4> const& P,
																						  ::boost::numeric::ublas::matrix_expression<MatrixExprT> const& L)
{
	typedef typename ::boost::multi_array<ValueT, 4>::size_type size_type;
	size_type const* sizes = P.shape();
	size_type nn = sizes[0];
	size_type nc = sizes[1];
	size_type n = nn*nc;
	matrix_type PP = reshape(P, n, n);
	vector_type LL = reshape(L, n, 1);

	// A = [ I_n - P ]
	A = ::boost::numeric::ublas::identity_matrix<value_type>(n) - PP;

	// b = lambda / \sum_i lambda_i
	b = LL / ::boost::numeric::ublasx::sum(LL);

	vector_type v(n);

	// Find V = A/b = (A'\b')'
	A = ::boost::numeric::ublas::trans(A);
	//b = ::boost::numeric::ublas::trans(b);
	size_type singular = ::boost::numeric::ublasx::lu_solve_inplace(A, b);

	matrix_type V = reshape(A, nn, nc);

	return V;
}
*/


template <typename ValueT, typename MatrixExprT>
typename ::boost::numeric::ublas::matrix_temporary_traits<MatrixExprT>::type visit_ratios(::boost::multi_array<ValueT, 4> const& P)
{
}


}}} // Namespace dcs::perfeval::qn


#endif // DCS_PERFEVAL_QN_OPERATION_VISIT_RATIOS_HPP
